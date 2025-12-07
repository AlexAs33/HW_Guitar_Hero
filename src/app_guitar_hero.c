/*
 *  VERSIÓN NO FUNCIONAL, SIMPLEMENTE ILUSTRA UNA IMPLEMENTACIÓN 
 *  DEL JUEGO GUITAR HERO CON UNA MÁQUINA DE ESTADOS
 */ 

#include "app_guitar_hero.h"

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
static int palpitaciones = 0;						//palipitaciones de leds secuencias
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
	e_SEC_INI,
    e_INICIO,
    e_SHOW_SEQUENCE,
    e_BEAT,
    e_TIMEOUT,
	e_SEC_FIN,
    e_FIN
} GH_STATE;
static GH_STATE estado_actual = e_SEC_INI;

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
		if (boton == TIMEOUT || !comprobar_acierto(boton)) {
				UART_LOG_INFO("A LA PROXIMA SERA...");

				//Fallo de no pulsar nada cuenta por dos
				if(boton == TIMEOUT) puntuacion -= FALLO * 2;
				else             puntuacion -= FALLO;

				racha = 0;
				fallos++;
		}
		else {
				UART_LOG_INFO("MUY BIEN!!");
								
				// Modificar puntuación dependiendo de la racha
				if(++racha >= 5) puntuacion += ACIERTO * racha;
				else             puntuacion += ACIERTO;
				aciertos++;    
		}
}

//------------------------------ ESTADO SECUENCIA INICIO ------------------------------//
void secuencia_inicio(EVENTO_T ev, uint32_t auxData){
    (void)ev;
    (void)auxData;
	if(palpitaciones < leds){
		drv_led_establecer(palpitaciones + 1, LED_ON);
		palpitaciones ++;
	}
	else{
		estado_actual = e_INICIO;
		encolar_EM(ev_GUITAR_HERO, 0);
	}
}

void iniciar_secuencia_inicio(){
	//inicar alarmas leds
	uint32_t alarma_sec_inicio = svc_alarma_codificar(true, T_SECS_INI_FIN, 0);
    svc_alarma_activar(alarma_sec_inicio, ev_SEC_INI_FIN, 0);
    svc_GE_suscribir(ev_SEC_INI_FIN, 0, secuencia_inicio);
}

//------------------------------ ESTADO DE INICIO ------------------------------//

void estado_inicio_gh(EVENTO_T ev, uint32_t auxData) {
    (void)ev;
    (void)auxData;

	drv_leds_apagar_todos();

	uint32_t flags_cancelar = svc_alarma_codificar(false, 0, 0);
	svc_alarma_activar(flags_cancelar, ev_SEC_INI_FIN, 0);
	svc_GE_cancelar(ev_SEC_INI_FIN, secuencia_inicio);

	estado_actual = e_SHOW_SEQUENCE;
	encolar_EM(ev_GUITAR_HERO, 0);
	UART_LOG_INFO("COMIENZA EL BEAT!! PODRAS SEGUIRLO?");
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
			estados_notas[0] = svc_random_value(min, max);;
		}

		drv_led_establecer((LED_id_t)1, BIT_TO_LED(estados_notas[0] & 0b10));
		drv_led_establecer((LED_id_t)2, BIT_TO_LED(estados_notas[0] & 0b01));

    //PREVIO
		drv_led_establecer((LED_id_t)3, BIT_TO_LED(estados_notas[1] & 0b10)); 
		drv_led_establecer((LED_id_t)4, BIT_TO_LED(estados_notas[1] & 0b01));

#ifdef DEBUG 
				svc_estadisticas_set_tmp(e_TERMINA_SECUENCIA);
#endif
		
    //JUEGO
    //esta codificado. usar estados_notas[2] para comprobar resultado
		notas_tocadas++;
		// He terminado la partitura
		if(notas_tocadas > TAM_PARTITURA + NOTAS_INIT){    
				encolar_EM(ev_GUITAR_HERO, 0);
        estado_actual = e_SEC_FIN;
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
			
				//codificar alarma timeout
				uint32_t flags_timeout = svc_alarma_codificar(false, PERIODO_LEDS - MARGEN_PULSAR, 0);  //tienes hasta el siguiente para darle
				svc_alarma_activar(flags_timeout, ev_TIMEOUT_LED, 0);
				estado_actual = e_BEAT;
		}
		// Programamos el siguiente compás
		uint32_t flags_boton = svc_alarma_codificar(false, PERIODO_LEDS, 0);  //tienes hasta el siguiente para darle
		svc_alarma_activar(flags_boton, ev_GUITAR_HERO, 0);
}

//------------------------------ ESTADO DE BOTONES ------------------------------//

void manejador_botones_guitar_hero(EVENTO_T evento, uint32_t auxData) {
			
		uint32_t id_boton = drv_botones_encontrar_indice(auxData);
	
    //cancelar alarma timeout
    uint32_t flags_cancelar = svc_alarma_codificar(false, 0, 0);
		svc_alarma_activar(flags_cancelar, ev_TIMEOUT_LED, 0);
	
    if(id_boton == 0 || id_boton == 1)	
				encolar_EM(ev_GUITAR_HERO, id_boton + 1);
    
    else if (id_boton == drv_botones_cantidad() - 1) {
        uint32_t flags = svc_alarma_codificar(false, T_RESET_MS, 0);
        svc_alarma_activar(flags, ev_FIN_GUITAR_HERO, 0);
    }
}

void estado_boton_guitar_hero(EVENTO_T evento, uint32_t auxData){
    (void) auxData; (void) evento;

	uint32_t boton;
	if (evento == ev_TIMEOUT_LED) {
		UART_LOG_DEBUG("TIMEOUT, APRIETA EL BOTON!!");
		boton = TIMEOUT; // No se ha pulsado ningún botón
	}
	else {
#ifdef DEBUG
		char buf[64];
		sprintf(buf, "Soy el botón %d", boton);
		UART_LOG_DEBUG(buf);
#endif
		boton = auxData;
	}
    modificar_puntuacion(boton);

    // Pasamos a representar siguiente compás
    estado_actual = e_SHOW_SEQUENCE;
}

//------------------------------ ESTADO SECUENCIA FIN ------------------------------//
void secuencia_fin(EVENTO_T ev, uint32_t auxData){
    (void)ev;
    (void)auxData;
	if(palpitaciones > 0){
		drv_led_establecer(palpitaciones, LED_ON);
		palpitaciones --;
	}
	else{
		estado_actual = e_FIN;
		encolar_EM(ev_GUITAR_HERO, 0);
	}
}

void iniciar_secuencia_fin(){
	//inicar alarmas leds
	uint32_t alarma_sec_inicio = svc_alarma_codificar(true, T_SECS_INI_FIN, 0);
    svc_alarma_activar(alarma_sec_inicio, ev_SEC_INI_FIN, 0);
    svc_GE_suscribir(ev_SEC_INI_FIN, 0, secuencia_fin);
}

//------------------------------ ESTADO DE FIN ------------------------------//

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

void estado_fin_partida_guitar_hero(EVENTO_T evento, uint32_t auxData){
    (void) evento;
    (void) auxData;
	
	drv_leds_apagar_todos();

	uint32_t flags_cancelar = svc_alarma_codificar(false, 0, 0);
	svc_alarma_activar(flags_cancelar, ev_SEC_INI_FIN, 0);
	svc_GE_cancelar(ev_SEC_INI_FIN, secuencia_fin);

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
    estado_actual = e_SEC_INI;
    encolar_EM(ev_GUITAR_HERO, 0);
}

//------------------------------ MÁQUINA DE ESTADOS ------------------------------//

void app_guitar_hero_actualizar(EVENTO_T ev, uint32_t aux)
{
    switch (estado_actual)
    {
		case e_SEC_INI:
			iniciar_secuencia_inicio();
			break;

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
						break;

		case e_SEC_FIN:
			iniciar_secuencia_fin();
			break;

        case e_FIN:
            estado_fin_partida_guitar_hero(ev, aux);
            break;
    }
}

//------------------------------ MAIN ------------------------------//

void app_guitar_hero_iniciar(unsigned int num_leds){
    leds = num_leds;

	svc_GE_suscribir(ev_PULSAR_BOTON, 0, manejador_botones_guitar_hero);
    svc_GE_suscribir(ev_FIN_GUITAR_HERO, 0, estado_fin_partida_guitar_hero);
    svc_GE_suscribir(ev_GUITAR_HERO, 0, app_guitar_hero_actualizar);
    svc_GE_suscribir(ev_TIMEOUT_LED, 0, estado_boton_guitar_hero);
	
		// Empezamos el juego
    encolar_EM(ev_GUITAR_HERO, 0);
    rt_GE_lanzador();
}
