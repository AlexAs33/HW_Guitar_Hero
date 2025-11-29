/*
 *  VERSIÓN NO FUNCIONAL, SIMPLEMENTE ILUSTRA UNA IMPLEMENTACIÓN 
 *  DEL JUEGO GUITAR HERO CON UNA MÁQUINA DE ESTADOS
 */ 

/*
#include "app_guitar_hero_FSM.h"

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

#include "random.h"

#include <stdio.h>
#include <stdlib.h>

//Monitores Overflow
#define MONITOR_CONSUMO 1
#define MONITOR_FIFO 2
#define MONITOR_GE 3

#define BIT_TO_LED(x) ((x) ? LED_ON : LED_OFF)

//----------DEFINICIONES DE STATICS----------
//Control de leds
static int leds = 0;                        //Numero de leds
static uint8_t estados_notas[3] = {0, 0, 0};  
//Control partida
static int notas_tocadas = 0; //Acordes que tiene una partitura
static int num_partidas = 0;                //num partidas jugadas
//Control puntuacion
static int puntuacion = 0;                  //puntuacion en una partida
static int aciertos = 0;                    //aciertos en una partida
static int fallos = 0;                      //fallos en una partida
static int puntuacion_total = 0;            //puntuacion total
static int racha = 0;                       //aciertos encadenados en una partida

//Partitura
#ifdef DEBUG
static uint8_t partitura[TAM_PARTITURA] = { 
    0b01, 0b10, 0b01, 0b10, 0b01};
#endif

// Estados Guitar Hero
typedef enum {
    e_INICIO,
    e_SHOW_SEQUENCE,
    e_BEAT,
    e_TIMEOUT,
    e_FIN
} GH_STATE;
static GH_STATE estado_actual = e_INICIO;

//------------------------------ AUXILIARES ------------------------------//

void encolar_EM(EVENTO_T evento, uint32_t auxData) {
		drv_sc_disable();
    rt_FIFO_encolar(evento, auxData);
		drv_sc_enable();
}

int comprobar_acierto(uint32_t boton)
{   return estados_notas[2] & (1 << (2 - boton));   }

//Pre: boton = 255 --> venimos de no haber pulsado boton
void modificar_puntuacion(uint32_t boton){
		//ver acierto / fallo y modificar puntuación
		if (boton == 255 || !comprobar_acierto(boton)) {
				UART_LOG_INFO("A LA PROXIMAS ERA...");

				//Fallo de no pulsar nada cuenta por dos
				if(boton == 255) puntuacion -= FALLO * 2;
				else             puntuacion -= FALLO;

				racha = 0;
				fallos++;
		}
		else {
				UART_LOG_INFO("MUY BIEN!!");
								
				// Modificar puntuación dependiendo de la racha
				if(++racha >= 5) puntuacion += ACIERTO * racha;
				else           puntuacion += ACIERTO;
				aciertos++;    
		}
}

//------------------------------ ESTADO DE INICIO ------------------------------//

void estado_inicio_gh(EVENTO_T ev, uint32_t auxData) {
    (void)ev;
    (void)auxData;

// Omitir secuencia de inicio en debug
#ifndef DEBUG
    for (int palpitaciones = 1; palpitaciones <= leds; palpitaciones++) {
        drv_led_establecer(palpitaciones, LED_ON);
        drv_tiempo_esperar_hasta_ms(drv_tiempo_actual_ms() + PERIODO_LEDS);
        drv_led_establecer(palpitaciones, LED_OFF);
    }

    drv_leds_encender_todos();
    drv_tiempo_esperar_hasta_ms(drv_tiempo_actual_ms() + PERIODO_LEDS); 
    drv_leds_apagar_todos();
		drv_tiempo_esperar_hasta_ms(drv_tiempo_actual_ms() + PERIODO_LEDS); 
#endif

    estado_actual = e_SHOW_SEQUENCE;
    encolar_EM(ev_GUITAR_HERO, 0);
}

//------------------------------ ESTADO SHIFT LEDS ------------------------------//

void estado_leds_guitar_hero(EVENTO_T evento, uint32_t auxData){
    (void) auxData; (void) evento;

    //shift desde iteracion anterior
    estados_notas[2] = estados_notas[1];
    estados_notas[1] = estados_notas[0];

    //FETCH
    if (notas_tocadas >= TAM_PARTITURA)
			estados_notas[0] = 0;

		else {
#ifdef DEBUG 
		estados_notas[0] = partitura[notas_tocadas];
#else
		estados_notas[0] = random_value(0, 2);
#endif 
		}

		drv_led_establecer((LED_id_t)1, BIT_TO_LED(estados_notas[0] & 0b10));
		drv_led_establecer((LED_id_t)2, BIT_TO_LED(estados_notas[0] & 0b01));

    //PREVIO
		drv_led_establecer((LED_id_t)3, BIT_TO_LED(estados_notas[1] & 0b10)); 
		drv_led_establecer((LED_id_t)4, BIT_TO_LED(estados_notas[1] & 0b01));

    //JUEGO
    //esta codificado. usar estados_notas[2] para comprobar resultado
		notas_tocadas++;
		// He terminado la partitura
		if(notas_tocadas > TAM_PARTITURA + NOTAS_INIT){    
				encolar_EM(ev_GUITAR_HERO, 0);
        estado_actual = e_FIN;
				return;
		}
		else if(notas_tocadas > NOTAS_INIT) {
			
#ifdef DEBUG
				char buf[64];
				sprintf(buf, "La partitura es %d", estados_notas[2]);
				UART_LOG_DEBUG(buf);
#endif
			
				//codificar alarma timeout
				uint32_t flags_timeout = svc_alarma_codificar(false, PERIODO_LEDS - MARGEN_PULSAR, 0);  //tienes hasta el siguiente para darle
				svc_alarma_activar(flags_timeout, ev_TIMEOUT_LED, 0);
				estado_actual = e_BEAT;
		}
		// Poblar los primeros compases
		else {
				estado_actual = e_SHOW_SEQUENCE;
				encolar_EM(ev_GUITAR_HERO, 0);
		}
		
		// Esperamos tiempo entre leds
		drv_tiempo_esperar_hasta_ms(drv_tiempo_actual_ms() + PERIODO_LEDS); 
}

//------------------------------ ESTADO DE BOTONES ------------------------------//

void manejador_botones_guitar_hero(int32_t id_pin, int32_t id_boton) {
    drv_botones_actualizar(ev_PULSAR_BOTON, id_pin);		//Actualiza el estado a bajo nivel del pin
    encolar_EM(ev_ACT_INACTIVIDAD, 0);
	
#ifdef DEBUG
		char buf[64];
		sprintf(buf, "Soy el botón %d", id_boton);
		UART_LOG_DEBUG(buf);
#endif
	
    if(id_boton == 0 || id_boton == 1)	
				encolar_EM(ev_GUITAR_HERO, id_boton + 1);
    
    else if (id_boton == drv_botones_cantidad() - 1) {
        uint32_t flags = svc_alarma_codificar(false, T_RESET_MS, 0);
        svc_alarma_activar(flags, ev_FIN_GUITAR_HERO, 0);
    }
}

void estado_boton_guitar_hero(EVENTO_T evento, uint32_t auxData){
    (void) auxData; (void) evento;

    drv_consumo_esperar();

    //cancelar alarma timeout
    uint32_t flags_cancelar = svc_alarma_codificar(false, 0, 0);
		svc_alarma_activar(flags_cancelar, ev_TIMEOUT_LED, 0);

    uint32_t boton = auxData;
	
#ifdef DEBUG
		char buf[64];
		sprintf(buf, "Soy el botón %d", boton);
		UART_LOG_DEBUG(buf);
#endif
					
    modificar_puntuacion(boton);

    // Pasamos a representar siguiente compás
    estado_actual = e_SHOW_SEQUENCE;
    encolar_EM(ev_GUITAR_HERO, 0);
}

//------------------------------ ESTADO DE TIMEOUT ------------------------------//

void estado_timeout_guitar_hero(EVENTO_T evento, uint32_t auxData){
    (void) auxData; (void) evento;
    UART_LOG_DEBUG("TIMOUT, APRIETA EL BOTON!!");
	
    // Informamos que el botón no se ha pulsado
    modificar_puntuacion(255);

    // Representamos siguiente compás
    estado_actual = e_SHOW_SEQUENCE;
    encolar_EM(ev_GUITAR_HERO, 0);
}

//------------------------------ ESTADO DE FIN ------------------------------//

//? solo esto o  separar de puntuacion
void estadisticas_guitar_hero(){
		drv_uart_puts("Tu desempegno en esta partida ha sido el siguiente\r\n");
		drv_uart_puts("Con "); drv_uart_putint(aciertos); drv_uart_puts(" aciertos\r\n");
		drv_uart_puts("Y "); drv_uart_putint(fallos); drv_uart_puts(" fallos\r\n");
		drv_uart_puts("Has conseguido una puntuacion de "); drv_uart_putint(puntuacion); drv_uart_puts(" puntos\r\n");
		drv_uart_puts("Lo que en tus "); drv_uart_putint(num_partidas); drv_uart_puts(" partidas, suma un total de "); drv_uart_putint(puntuacion_total); drv_uart_puts(" puntos\r\n");
}

void estado_fin_partida_guitar_hero(EVENTO_T evento, uint32_t auxData){
    (void) evento;
    (void) auxData;

    num_partidas ++;
    puntuacion_total += puntuacion;
		notas_tocadas = 0;

    estadisticas_guitar_hero();  
		aciertos = 0;
		fallos = 0;
		puntuacion = 0;
	  // Reiniciamos estado de las notas
		estados_notas[0] = 0;
		estados_notas[1] = 0;
		estados_notas[2] = 0;	
	
    // Volvemos a empezar una nueva partida
    estado_actual = e_INICIO;
    encolar_EM(ev_GUITAR_HERO, 0);
}

//------------------------------ MÁQUINA DE ESTADOS ------------------------------//

void estados_guitar_hero(EVENTO_T ev, uint32_t aux)
{
    switch (estado_actual)
    {
        case e_INICIO:
            estado_inicio_gh(ev, aux);
            break;

        case e_SHOW_SEQUENCE:
            estado_leds_guitar_hero(ev, aux);
            break;

        case e_BEAT:
            estado_boton_guitar_hero(ev, aux);
            break;
				
				case e_TIMEOUT: 
						estado_timeout_guitar_hero(ev, aux);
						break;

        case e_FIN:
            estado_fin_partida_guitar_hero(ev, aux);
            break;
				
    }
}

//------------------------------ MAIN ------------------------------//

void guitar_hero(unsigned int num_leds){
    leds = num_leds;

    //Poner en marcha lo necesario del background
    drv_monitor_iniciar();
    drv_consumo_iniciar((MONITOR_id_t)MONITOR_CONSUMO);
    rt_FIFO_inicializar((MONITOR_id_t)MONITOR_FIFO);  
    rt_GE_iniciar((MONITOR_id_t)MONITOR_GE);
    random_iniciar(drv_tiempo_actual_us());

    drv_wdt_iniciar(PERIODO_WDT);

    drv_botones_iniciar(manejador_botones_guitar_hero);

    svc_GE_suscribir(ev_FIN_GUITAR_HERO, 0, estado_fin_partida_guitar_hero);
    svc_GE_suscribir(ev_GUITAR_HERO, 0, estados_guitar_hero);
    svc_GE_suscribir(ev_TIMEOUT_LED, 0, estado_timeout_guitar_hero);
		
    drv_uart_init(9600);
	
		// Empezamos el juego
    encolar_EM(ev_GUITAR_HERO, 0);
    rt_GE_lanzador();
}
*/
