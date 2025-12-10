/* ***************************************************************************************
 * P.H.2025: Implementación del Modo Guitar Hero
 * Contiene la lógica completa del juego Guitar Hero basada en una máquina de
 * estados. Gestiona la secuencia inicial y final, el control de dificultad,
 * la generación y visualización de notas, el registro de aciertos y fallos,
 * así como el cálculo de la puntuación y las estadísticas finales.
 *
 * Este módulo coordina:
 * - La representación de leds y el desplazamiento de notas
 * - La detección de pulsaciones de botones y validación de aciertos
 * - El tratamiento de tiempos, alarmas y eventos del sistema
 * - La evolución entre los distintos estados del juego
 *
 * Autores:
 * - Pablo Plumed
 * - Alex Asensio
 *************************************************************************************** */

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

#define BIT_TO_LED(x) ((x) ? LED_ON : LED_OFF)

//----------DEFINICIONES DE STATICS----------
//Control de leds
static volatile int leds = 0;                        //Numero de leds
static int min = 1, max = 2;
static uint8_t estados_notas[3] = {0, 0, 0};  
static int periodo_leds = 0;
//Control partida
//static int notas_tocadas = 0; //Acordes que tiene una partitura
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
// Muestra la secuencia inicial encendiendo leds en orden
bool sec_inicio(uint32_t auxData) {
    bool mostrar_secuencia = (auxData <= leds);
    if (mostrar_secuencia) {
        if (auxData == leds)
            drv_leds_encender_todos();
        else {
            drv_led_establecer(auxData + 1, LED_ON);
            drv_led_establecer(auxData, LED_OFF);
        }
    }
    return mostrar_secuencia;
}

// Desplaza el estado de leds y genera una nueva nota aleatoria
void shift_leds(uint32_t notas_tocadas) {
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

// Comprueba si el botón pulsado coincide con la nota esperada
int comprobar_acierto(uint32_t boton)
{   return estados_notas[2] & (1 << (2 - boton));   }

// Actualiza la puntuación según acierto o fallo
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

// Muestra por la uart las estadísticas de la partida
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

// Reinicia todas las variables del juego y aumenta la dificultad
void reiniciar_variables_juego() {
    num_partidas ++;
    puntuacion_total += puntuacion;
    //notas_tocadas = 0;
    min = 1; max = 2;

    aciertos = 0;
    fallos = 0;
    puntuacion = 0;
    
    // Reiniciamos estado de las notas
    estados_notas[0] = 0;
    estados_notas[1] = 0;
    estados_notas[2] = 0;

    //aumento dificultad
    if(periodo_leds > MARGEN_PULSAR * 2)
        periodo_leds /= 2;
}

// Secuencia final parpadeando todos los leds
bool sec_fin(uint32_t auxData) {
    bool mostrar_secuencia = (auxData < leds);
    if (mostrar_secuencia) {
        if (auxData % 2) drv_leds_apagar_todos();
        else drv_leds_encender_todos();
    }
    return mostrar_secuencia;
}

//------------------------------ MÁQUINA DE ESTADOS ------------------------------//

void app_guitar_hero_actualizar(EVENTO_T ev, uint32_t auxData)
{
    switch (estado_actual)
    {
        //------------------------------ ESTADO DE INICIO ------------------------------//

        case e_INICIO:
            if (ev == ev_GUITAR_HERO) 
            {
                    // Se está representando la secuencia de inicio
                    if(sec_inicio(auxData)){
                        uint32_t flags_cancelar = svc_alarma_codificar(false, T_SECS_INI_FIN, 0);
                        svc_alarma_activar(flags_cancelar, ev_GUITAR_HERO, auxData + 1);
                    }
					// Cuando se ha terminado de representar empezamos el juego
                    else {
                        drv_leds_apagar_todos();

                        UART_LOG_INFO(MENU_DIFICULTAD);
                        estado_actual = e_ELEG_DIFIC;
                        rt_FIFO_encolar(ev_GUITAR_HERO, 0);
                    }
            }
            break;

        //------------------------------ ESTADO ELECCIÓN DIFICULTAD ------------------------------//

        case e_ELEG_DIFIC:
            if (ev == ev_PULSAR_BOTON) 
            {
                uint32_t id_boton = drv_botones_encontrar_indice(auxData);
                
                // Dificultad media y superiores son con notas nulas
                min -= (id_boton > 0);
                // Dificultad alta y superior añade notas dobles
                max += (id_boton > 1);
                // Ültima dificultad, se duplica la velocidad
                if (id_boton > 3) 
                    periodo_leds /= 2;

                estado_actual = e_SHOW_SEQUENCE;
                rt_FIFO_encolar(ev_GUITAR_HERO, 0);
                UART_LOG_INFO("COMIENZA EL BEAT!! PODRAS SEGUIRLO?");
            }
            break;

        //------------------------------ ESTADO SHOW SECUENCE ------------------------------//

        case e_SHOW_SEQUENCE:
            if (ev == ev_GUITAR_HERO)
            {
#ifdef DEBUG 
                svc_estadisticas_set_tmp(e_TERMINA_SECUENCIA);
#endif  
                // actualizar estado del juego
                shift_leds(auxData); // auxData = numero de notas tocadas

                // He terminado la partitura
                if(auxData > TAM_PARTITURA + NOTAS_INIT){    
                    rt_FIFO_encolar(ev_GUITAR_HERO, 0);
                    estado_actual = e_FIN;
                    UART_LOG_INFO("FIN DE LA PARTIDA, TE HAS VENTILADO LA PARTITURA");
                    return;
                }
                else if (auxData <= NOTAS_INIT) {
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
                    uint32_t flags_timeout = svc_alarma_codificar(false, periodo_leds - MARGEN_PULSAR, 0);
                    svc_alarma_activar(flags_timeout, ev_TIMEOUT_LED, 0);
                    estado_actual = e_BEAT;
                }
                // Programamos el siguiente compás
                svc_alarma_activar(svc_alarma_codificar(false, periodo_leds, 0), ev_GUITAR_HERO, auxData + 1);
            }
        break;

        //------------------------------ ESTADO DE BOTONES ------------------------------//

        case e_BEAT:
            // Si no se ha pulsado ningún botón a tiempo se notifica 
            if (ev == ev_TIMEOUT_LED) 
            {
                modificar_puntuacion(TIMEOUT);
            }
            // Si se ha pulsado un botón
            else if (ev == ev_PULSAR_BOTON) 
            {
                uint32_t id_boton = drv_botones_encontrar_indice(auxData);
                
                // Solo procesar botones 0 y 1 (notas)
                if(id_boton == 0 || id_boton == 1) {
                    // Cancelar alarma timeout
                    uint32_t flags_cancelar = svc_alarma_codificar(false, 0, 0);
                    svc_alarma_activar(flags_cancelar, ev_TIMEOUT_LED, 0);
#ifdef DEBUG
                    char buf[64];
                    sprintf(buf, "Soy el botón %d", id_boton);
                    UART_LOG_DEBUG(buf);
#endif
                    // Se comprueba el acierto
                    modificar_puntuacion(id_boton + 1);
                }
                // Verificar si es el botón de reset
                else if (id_boton == drv_botones_cantidad() - 1) {
                    uint32_t flags = svc_alarma_codificar(false, T_RESET_MS, 0);
                    svc_alarma_activar(flags, ev_FIN_GUITAR_HERO, 0);
                    return;
                } 
            }
            // Pasamos a representar siguiente compás
            estado_actual = e_SHOW_SEQUENCE;
            break;

        case e_TIMEOUT: 
            // Estado no utilizado actualmente
            break;

        //------------------------------ ESTADO DE FIN ------------------------------//

        case e_FIN:
            if (ev == ev_GUITAR_HERO || ev == ev_FIN_GUITAR_HERO) 
            {   
                // Se está representando la secuencia de fin
                if (sec_fin(auxData)) {
                    uint32_t flags_cancelar = svc_alarma_codificar(false, T_SECS_INI_FIN, 0);
                    svc_alarma_activar(flags_cancelar, ev_GUITAR_HERO, auxData + 1);
                }
                else {
                    // Mostrar estadísticas de la partida
                    estadisticas_guitar_hero();  

                    // Se establecen los valores inicialies
                    reiniciar_variables_juego();
                    
                    // Volvemos a empezar una nueva partida
                    estado_actual = e_INICIO;
                    rt_FIFO_encolar(ev_GUITAR_HERO, 0);
                }
            }
            break;
    }
}

//------------------------------ INICIAR ------------------------------//

void app_guitar_hero_iniciar(unsigned int num_leds){
    leds = num_leds;

    // Todas las suscripciones apuntan a la función actualizar
    svc_GE_suscribir(ev_PULSAR_BOTON, 0, app_guitar_hero_actualizar);
    svc_GE_suscribir(ev_FIN_GUITAR_HERO, 0, app_guitar_hero_actualizar);
    svc_GE_suscribir(ev_GUITAR_HERO, 0, app_guitar_hero_actualizar);
    svc_GE_suscribir(ev_TIMEOUT_LED, 0, app_guitar_hero_actualizar);
    
    // Empezamos el juego
    periodo_leds = PERIODO_LEDS;
    rt_FIFO_encolar(ev_GUITAR_HERO, 0);
    rt_GE_lanzador();
}
