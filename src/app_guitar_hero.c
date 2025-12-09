/*
 *  VERSIÓN NO FUNCIONAL, SIMPLEMENTE ILUSTRA UNA IMPLEMENTACIÓN 
 *  DEL JUEGO GUITAR HERO CON UNA MÁQUINA DE ESTADOS
 */ 

#include "app_guitar_hero.h"
#include "app_guitar_hero_config.h"

#include "rt_fifo.h"
#include "rt_GE.h"

#include "svc_GE.h"
#include "svc_alarma.h"

#include "drv_botones.h"
#include "drv_consumo.h"
#include "drv_leds.h"
#include "drv_wdt.h"
#include "drv_uart.h"
#include "drv_sc.h"

#include "svc_random.h"
#include <stdio.h>

#ifdef DEBUG
    #include "svc_estadisticas.h"
#endif

#define TIMEOUT 255

#define BIT_TO_LED(x) ((x) ? LED_ON : LED_OFF)

//----------DEFINICIONES DE STATICS----------
//Control de leds
static volatile int leds = 0;                        //Numero de leds
static int min = 1, max = 2;
static uint8_t estados_notas[3] = {0, 0, 0};  
static int periodo_leds = 0;
//Control partida
static int notas_tocadas = 0; //Acordes que tiene una partitura
static int num_partidas = 0;                //num partidas jugadas
//Control puntuacion
static int puntuacion = 0;                  //puntuacion en una partida
static int aciertos = 0;                    //aciertos en una partida
static int fallos = 0;                      //fallos en una partida
static int puntuacion_total = 0;            //puntuacion total
static int racha = 0;                       //aciertos encadenados en una partida

// Estados Guitar Hero
typedef enum {
    e_INICIO,
    e_ELEG_DIFIC,
    e_SHOW_SEQUENCE,
    e_BEAT,
    e_TIMEOUT,
    e_FIN
} GH_STATE;
static GH_STATE estado_actual = e_INICIO;

//------------------------------ AUXILIARES ------------------------------//

void encolar_EM(EVENTO_T evento, uint32_t auxData) {
    //drv_sc_disable();
    rt_FIFO_encolar(evento, auxData);
    //drv_sc_enable();
}

int comprobar_acierto(uint32_t boton)
{   return estados_notas[2] & (1 << (2 - boton));   }

void shift_leds() {
    // shift desde iteracion anterior
    estados_notas[2] = estados_notas[1];
    estados_notas[1] = estados_notas[0];

    // fetch de nueva nota
    if (notas_tocadas >= TAM_PARTITURA)
        estados_notas[0] = 0;
    else 
        estados_notas[0] = svc_random_value(min, max);
    
    // actualizar primera fila
    drv_led_establecer((LED_id_t)1, BIT_TO_LED(estados_notas[0] & 0b10));
    drv_led_establecer((LED_id_t)2, BIT_TO_LED(estados_notas[0] & 0b01));

    // actualizar siguiente fila
    drv_led_establecer((LED_id_t)3, BIT_TO_LED(estados_notas[1] & 0b10)); 
    drv_led_establecer((LED_id_t)4, BIT_TO_LED(estados_notas[1] & 0b01));
}

//Pre: boton = 255 --> venimos de no haber pulsado boton
void modificar_puntuacion(uint32_t boton){
    //ver acierto / fallo y modificar puntuación
    if (comprobar_acierto(boton) || (estados_notas[2] == 0 && boton == TIMEOUT)) {
        UART_LOG_INFO("MUY BIEN!!");
                        
        // Modificar puntuación dependiendo de la racha
        if(++racha >= 5) puntuacion += ACIERTO * racha;
        else             puntuacion += ACIERTO;
        aciertos++;    
    }
    else {
        if (boton == TIMEOUT)
                    UART_LOG_DEBUG("APRIETA EL BOTON!!");
        UART_LOG_INFO("A LA PROXIMA SERA...");

        //Fallo de no pulsar nada cuenta por dos
        if(boton == TIMEOUT) puntuacion -= FALLO * 2;
        else             puntuacion -= FALLO;

        racha = 0;
        fallos++;
    }
}

void estadisticas_guitar_hero(){
    drv_uart_puts("Tu desempegno en esta partida ha sido el siguiente\r\n");
    drv_uart_puts("Con "); drv_uart_putint(aciertos); drv_uart_puts(" aciertos\r\n");
    drv_uart_puts("Y "); drv_uart_putint(fallos); drv_uart_puts(" fallos\r\n");
    drv_uart_puts("Has conseguido una puntuacion de "); drv_uart_putint(puntuacion); drv_uart_puts(" puntos\r\n");
    drv_uart_puts("Lo que en tus "); drv_uart_putint(num_partidas); drv_uart_puts(" partidas, suma un total de "); drv_uart_putint(puntuacion_total); drv_uart_puts(" puntos\r\n");
    
#ifdef DEBUG
    svc_estadisticas_print();
#endif
}

void reiniciar_variables_juego() {
    num_partidas ++;
    puntuacion_total += puntuacion;
    notas_tocadas = 0;
    min = 1; max = 2;

    estadisticas_guitar_hero();  
    aciertos = 0;
    fallos = 0;
    puntuacion = 0;
    
    // Reiniciamos estado de las notas
    estados_notas[0] = 0;
    estados_notas[1] = 0;
    estados_notas[2] = 0;
}

//------------------------------ ESTADO DE INICIO ------------------------------//

void estado_inicio_gh(EVENTO_T ev, uint32_t auxData) {
    (void)ev;
    (void)auxData;
    static int palpitaciones = 0;

    if(palpitaciones <= leds){
        if (palpitaciones == leds)
            drv_leds_encender_todos();
        else {
            drv_led_establecer(palpitaciones + 1, LED_ON);
            drv_led_establecer(palpitaciones, LED_OFF);
        }
        palpitaciones ++;

        uint32_t flags_cancelar = svc_alarma_codificar(false, T_SECS_INI_FIN, 0);
        svc_alarma_activar(flags_cancelar, ev_GUITAR_HERO, 0);
    }
    else {
        drv_leds_apagar_todos();

        palpitaciones = 0;
        estado_actual = e_ELEG_DIFIC;
        encolar_EM(ev_GUITAR_HERO, 0);
        UART_LOG_INFO("PULSA UN BOTON PARA ELEGIR DIFICULTAD: \n\
       BOTON 1 - FACILITO\n\
       BOTON 2 - STANDARD\n\
       BOTON 3 - LA COSA SE PONE SERIA...\n\
       BOTON 4 - PARA VERDADEROS BEAT HEROS!\n");
    }
}

//------------------------------ ESTADO ELECCIÓN DIFICULTAD ------------------------------//

void estado_eleg_dific_gh(EVENTO_T evento, uint32_t auxData) {
    if (evento == ev_PULSAR_BOTON) {
        uint32_t id_boton = drv_botones_encontrar_indice(auxData);
        
        min -= (id_boton > 0);
        max += (id_boton > 1);
        if (id_boton > 3) 
            periodo_leds /= 2;

        estado_actual = e_SHOW_SEQUENCE;
        encolar_EM(ev_GUITAR_HERO, 0);
        UART_LOG_INFO("COMIENZA EL BEAT!! PODRAS SEGUIRLO?");
    }
}

//------------------------------ ESTADO SHIFT LEDS ------------------------------//

void estado_leds_guitar_hero(EVENTO_T evento, uint32_t auxData){
    (void) auxData; (void) evento;
    
    // actualizar estado del juego
    shift_leds();

#ifdef DEBUG 
    svc_estadisticas_set_tmp(e_TERMINA_SECUENCIA);
#endif
    
    notas_tocadas++;
    // He terminado la partitura
    if(notas_tocadas > TAM_PARTITURA + NOTAS_INIT){    
        encolar_EM(ev_GUITAR_HERO, 0);
        estado_actual = e_FIN;
        UART_LOG_INFO("FIN DE LA PARTIDA, TE HAS VENTILADO LA PARTITURA");
        return;
    }
    else if (notas_tocadas <= NOTAS_INIT) {
        estado_actual = e_SHOW_SEQUENCE;
    }
    else {		
#ifdef DEBUG
        char buf[64];
        sprintf(buf, "La partitura es %d", estados_notas[2]);
        UART_LOG_DEBUG(buf);
#endif
        UART_LOG_INFO("DALE AL BEAT");
        
        //codificar alarma timeout
        uint32_t flags_timeout = svc_alarma_codificar(false, periodo_leds - MARGEN_PULSAR, 0);  //tienes hasta el siguiente para darle
        svc_alarma_activar(flags_timeout, ev_TIMEOUT_LED, 0);
        estado_actual = e_BEAT;
    }
    // Programamos el siguiente compás
    uint32_t flags_boton = svc_alarma_codificar(false, periodo_leds, 0);  //tienes hasta el siguiente para darle
    svc_alarma_activar(flags_boton, ev_GUITAR_HERO, 0);
}

//------------------------------ ESTADO DE BOTONES ------------------------------//

void estado_boton_guitar_hero(EVENTO_T evento, uint32_t auxData){
    (void) auxData; (void) evento;

    uint32_t boton;
    if (evento == ev_TIMEOUT_LED) {
        boton = TIMEOUT; // No se ha pulsado ningún botón
    }
    else if (evento == ev_PULSAR_BOTON) {
        uint32_t id_boton = drv_botones_encontrar_indice(auxData);
        
        // Verificar si es el botón de reset
        if (id_boton == drv_botones_cantidad() - 1) {
            uint32_t flags = svc_alarma_codificar(false, T_RESET_MS, 0);
            svc_alarma_activar(flags, ev_FIN_GUITAR_HERO, 0);
            return;
        }
        
        // Solo procesar botones 0 y 1 (notas)
        if(id_boton == 0 || id_boton == 1) {
            //cancelar alarma timeout
            uint32_t flags_cancelar = svc_alarma_codificar(false, 0, 0);
            svc_alarma_activar(flags_cancelar, ev_TIMEOUT_LED, 0);

#ifdef DEBUG
            char buf[64];
            sprintf(buf, "Soy el botón %d", boton);
            UART_LOG_DEBUG(buf);
#endif
            boton = id_boton + 1;
        }
        else {
            return; // Ignorar otros botones
        }
    }
    else {
        return; // Evento no esperado
    }
    
    modificar_puntuacion(boton);

    // Pasamos a representar siguiente compás
    estado_actual = e_SHOW_SEQUENCE;
}

//------------------------------ ESTADO DE FIN ------------------------------//

void estado_fin_partida_guitar_hero(EVENTO_T evento, uint32_t auxData){
    (void) evento;
    (void) auxData;
    
    static int palpitaciones = 0;
    if (palpitaciones < leds) {
        if (palpitaciones++ % 2) drv_leds_apagar_todos();
        else drv_leds_encender_todos();
        
        uint32_t flags_cancelar = svc_alarma_codificar(false, T_SECS_INI_FIN, 0);
        svc_alarma_activar(flags_cancelar, ev_GUITAR_HERO, 0);
    }
    else {
        palpitaciones = 0;
        // se establecen los valores inicialies
        reiniciar_variables_juego();
            
        //aumento dificultad
        if(periodo_leds > MARGEN_PULSAR * 2)
            periodo_leds /= 2;
        
        // Volvemos a empezar una nueva partida
        estado_actual = e_INICIO;
        encolar_EM(ev_GUITAR_HERO, 0);
    }
}

//------------------------------ MÁQUINA DE ESTADOS ------------------------------//

void app_guitar_hero_actualizar(EVENTO_T ev, uint32_t aux)
{
    switch (estado_actual)
    {
        case e_INICIO:
            if (ev == ev_GUITAR_HERO) {
                estado_inicio_gh(ev, aux);
            }
            break;

        case e_ELEG_DIFIC:
            if (ev == ev_PULSAR_BOTON) {
                estado_eleg_dific_gh(ev, aux);
            }
            break;

        case e_SHOW_SEQUENCE:
            if (ev == ev_GUITAR_HERO) {
                estado_leds_guitar_hero(ev, aux);
            }
            break;

        case e_BEAT:
            if (ev == ev_PULSAR_BOTON || ev == ev_TIMEOUT_LED) {
                estado_boton_guitar_hero(ev, aux);
            }
            break;

        case e_TIMEOUT: 
            // Estado no utilizado actualmente
            break;

        case e_FIN:
            if (ev == ev_GUITAR_HERO || ev == ev_FIN_GUITAR_HERO) {
                estado_fin_partida_guitar_hero(ev, aux);
            }
            break;
    }
}

//------------------------------ MAIN ------------------------------//

void app_guitar_hero_iniciar(unsigned int num_leds){
    leds = num_leds;

    // Todas las suscripciones apuntan a la función actualizar
    svc_GE_suscribir(ev_PULSAR_BOTON, 0, app_guitar_hero_actualizar);
    svc_GE_suscribir(ev_FIN_GUITAR_HERO, 0, app_guitar_hero_actualizar);
    svc_GE_suscribir(ev_GUITAR_HERO, 0, app_guitar_hero_actualizar);
    svc_GE_suscribir(ev_TIMEOUT_LED, 0, app_guitar_hero_actualizar);
    
    // Empezamos el juego
    periodo_leds = PERIODO_LEDS;
    encolar_EM(ev_GUITAR_HERO, 0);
    rt_GE_lanzador();
}
