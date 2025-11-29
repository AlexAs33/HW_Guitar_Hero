/* ***************************************************************************************
 * P.H.2025: Implementacion del juego Guitar Hero
 *
 * Autores:
 * - Pablo Plumed
 * - Alex Asensio
 *************************************************************************************** */

#include "app_guitar_hero.h"

#include "rt_fifo.h"
#include "rt_GE.h"

#include "svc_GE.h"
#include "svc_alarma.h"

#include "drv_botones.h"
#include "drv_consumo.h"
#include "drv_leds.h"
#include "drv_wdt.h"
#include "drv_sc.h"
#include "drv_uart.h"

#include "random.h"

#include <stdio.h>
#include <stdlib.h>

//Monitores Overflow
#define MONITOR_CONSUMO 1
#define MONITOR_FIFO 2
#define MONITOR_GE 3

#define BIT_TO_LED(x) ((x) ? LED_ON : LED_OFF)

//------------------------------ DEFINICIONES DE VAIRABLES ------------------------------//
//Control de leds
static int leds = 0;                        //Numero de leds
static int periodo_leds = 0;                //ms cada los que se encienden los leds
static uint8_t estados_notas[3] = {0, 0, 0};  
//Control partida
static int notas_tocadas = 0;               //Acordes que tiene una partitura
static int en_partida = 0;                  // 1 en partida
static int num_partidas = 0;                //num partidas jugadas
//Control puntuacion
static int puntuacion = 0;                  //puntuacion en una partida
static int aciertos = 0;                    //aciertos en una partida
static int fallos = 0;                      //fallos en una partida
static int puntuacion_total = 0;            //puntuacion total
static int racha = 0;                       //aciertos encadenados en una partida
static int palpitaciones = 0;

// PARTITURA
#ifdef DEBUG
static uint8_t partitura[TAM_PARTITURA] = { 
    0b10, 0b01, 0b10, 0b01, 0b10};
#endif

void sec_inicio_gh(EVENTO_T ev, uint32_t auxData);
void sec_fin_gh(EVENTO_T ev, uint32_t auxData);
void partida_guitar_hero(void);

//------------------------------ AUXILIARES ------------------------------//
//Pre modo = 0 --> inicio. 1 --> fin
void main_secuencias_gh(int modo){
    uint32_t alarma_sec_inicio = svc_alarma_codificar(true, SEC_INI_FIN, 0);
    svc_alarma_activar(alarma_sec_inicio, ev_SEC_INI_FIN, 0);
    palpitaciones = 0;
    if(modo == 0) svc_GE_suscribir(ev_SEC_INI_FIN, 0, sec_inicio_gh);
    else if (modo == 1) svc_GE_suscribir(ev_SEC_INI_FIN, 0, sec_fin_gh);
}

void encolar_EM(EVENTO_T evento, uint32_t auxData) {
		drv_sc_disable();
    rt_FIFO_encolar(evento, auxData);
		drv_sc_enable();
}

//Cálculos de evento
int comprobar_acierto(uint32_t boton){
    return estados_notas[2] & (1 << (2 - boton));
}

//Pre: boton = 255 --> venimos de no haber pulsado boton
void modificar_puntuacion(uint32_t boton){
	//ver acierto / fallo y modificar puntuación
	if(boton == 255 || !comprobar_acierto(boton)){
				UART_LOG_INFO("A LA PROXIMA SERA...");
        racha = 0;
        fallos++;
         //Fallo de no pulsar nada cuenta por dos
        if(boton == 255)  puntuacion -= FALLO * 2;
        else              puntuacion -= FALLO;
	}
	else {
				UART_LOG_INFO("MUY BIEN!!");
        if(++racha >= 5)  puntuacion += ACIERTO * racha;
        else              puntuacion += ACIERTO;
        aciertos++;    
	}
}


//------------------------------ ESTADO DE INICIO ------------------------------//

void sec_inicio_gh(EVENTO_T ev, uint32_t auxData){   
		(void)ev;
		(void)auxData;

    if (palpitaciones < leds){
        if (palpitaciones > 0) drv_led_establecer(palpitaciones, LED_OFF);
        drv_led_establecer(palpitaciones + 1, LED_ON);
        palpitaciones++;
    }
    else if (palpitaciones == leds) {
        drv_leds_encender_todos();
				palpitaciones++;
		}
    else {
        uint32_t flags_cancelar = svc_alarma_codificar(false, 0, 0);
				svc_alarma_activar(flags_cancelar, ev_SEC_INI_FIN, 0);
				svc_GE_cancelar(ev_SEC_INI_FIN, sec_inicio_gh);
        for(int i = 1; i <= leds; i++)
						drv_led_establecer(i, LED_OFF);
				en_partida = 1;
				//activar alarma --> periodo_leds
				uint32_t alarmaGH = svc_alarma_codificar(true, periodo_leds, 0);
				svc_alarma_activar(alarmaGH, ev_LEDS_GUITAR_HERO, 0);
    }
}

//------------------------------ ESTADO SHIFT LEDS ------------------------------//

/*registro estados_notas
    estados_notas[0] --> nota que se obtine y enciende 1 || 2
    estados_notas[1] --> enciende 3 || 4
    estados_notas[2] --> lo que se debe pulsar

    codif: 00, 01, 10, 11 --> bit + signf leds 1,3 boton 1
                          --> bit - signf leds 2,4 boton 2
*/
/**
(led 1) (led 2)     FETCH   0
(led 3) (led 4)     PREVIO  1
(bot 1) (bot 2)     JUEGO   2
**/
void evento_leds_guitar_hero(EVENTO_T evento, uint32_t auxData){
    (void) auxData; (void) evento;

    //shift desde iteracion anterior
    estados_notas[2] = estados_notas[1];
    estados_notas[1] = estados_notas[0];

    //FETCH
    if (notas_tocadas >= TAM_PARTITURA)
			estados_notas[0] = 0;

#ifdef DEBUG 
		 estados_notas[0] = partitura[notas_tocadas];
#else
		 estados_notas[0] = random_value(0, 3);
#endif 

		drv_led_establecer((LED_id_t)1, BIT_TO_LED(estados_notas[0] & 0b10));
		drv_led_establecer((LED_id_t)2, BIT_TO_LED(estados_notas[0] & 0b01));

    //PREVIO
		drv_led_establecer((LED_id_t)3, BIT_TO_LED(estados_notas[1] & 0b10)); 
		drv_led_establecer((LED_id_t)4, BIT_TO_LED(estados_notas[1] & 0b01));

    //JUEGO
    //esta codificado. usar estados_notas[2] para comprobar resultado
		notas_tocadas++;
		if(notas_tocadas > 2){
			
#ifdef DEBUG
				char buf[64];
        sprintf(buf, "La partitura es %d", estados_notas[2]);
        UART_LOG_DEBUG(buf);
#endif
			
			//codificar alarma timeout
			uint32_t flags_timeout = svc_alarma_codificar(false, periodo_leds - MARGEN_PULSAR, 0);  //tienes hasta el siguiente para darle
			svc_alarma_activar(flags_timeout, ev_TIMEOUT_LED, 0);
		}
		
		if(notas_tocadas > TAM_PARTITURA + 2)
      encolar_EM(ev_FIN_GUITAR_HERO, 0);
}

//------------------------------ ESTADO DE BOTONES ------------------------------//

void manejador_interrupcion_botones_fin(int32_t id_pin, int32_t id_boton) {
		sec_fin_gh((EVENTO_T) 0, 0);
		while(1); 
}

void manejador_interrupcion_botones_juego(int32_t id_pin, int32_t id_boton) {
    drv_botones_actualizar(ev_PULSAR_BOTON, id_pin);		//Actulaiza el estado a bajo nivel del pin
    encolar_EM(ev_ACT_INACTIVIDAD, 0);
	
    if(id_boton == 0 || id_boton == 1)
        encolar_EM(ev_BOTONES_GUITAR_HERO, id_boton + 1); 	//Produce un evento del juego asociado al botón.
    else if (id_boton == drv_botones_cantidad() - 1) {   
        uint32_t flags = svc_alarma_codificar(false, T_RESET_MS, 0);
        svc_alarma_activar(flags, ev_FIN_GUITAR_HERO, 0);
    }
}

void manejador_interrupcion_botones_gh(int32_t id_pin, int32_t id_boton) {
    if (id_boton != -1) {
        if(en_partida) manejador_interrupcion_botones_juego(id_pin, id_boton);

        else manejador_interrupcion_botones_fin(id_pin, id_boton);
    }
}

void evento_boton_guitar_hero(EVENTO_T evento, uint32_t auxData){
    (void) auxData; (void) evento;
	
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
}

//------------------------------ ESTADO DE TIMEOUT ------------------------------//

void evento_timeout_guitar_hero(EVENTO_T evento, uint32_t auxData){
    (void) auxData; (void) evento;
		UART_LOG_DEBUG("TIMOUT, APRIETA EL BOTON!!");
	
    modificar_puntuacion(255);
    //cancelar alarma
    uint32_t flags_cancelar = svc_alarma_codificar(false, 0, 0);
    svc_alarma_activar(flags_cancelar, ev_TIMEOUT_LED, 0);
}

//------------------------------ ESTADO DE FIN ------------------------------//

void sec_fin_gh(EVENTO_T ev, uint32_t auxData){
    for(int i = leds; i >= 1; i--) {
			drv_led_establecer(i, LED_ON);
			for (int j = 0; j <= SEC_INI_FIN; j++)
					drv_consumo_esperar();
		}
		
		drv_leds_apagar_todos();
}

void estadisticas_guitar_hero(){
		drv_uart_puts("Tu desempegno en esta partida ha sido el siguiente\r\n");
		drv_uart_puts("Con "); drv_uart_putint(aciertos); drv_uart_puts(" aciertos\r\n");
		drv_uart_puts("Y "); drv_uart_putint(fallos); drv_uart_puts(" fallos\r\n");

		drv_uart_puts("Has conseguido una puntuacion de "); drv_uart_putint(puntuacion); drv_uart_puts(" puntos\r\n");
		drv_uart_puts("Lo que en tus "); drv_uart_putint(num_partidas); drv_uart_puts(" partidas, suma un total de "); drv_uart_putint(puntuacion_total); drv_uart_puts(" puntos\r\n");
}

void fin_partida_guitar_hero(EVENTO_T evento, uint32_t auxData){
    (void) evento;
    (void) auxData;
	
		// Matar alarma ev_LEDS_GUITAR_HERO
    uint32_t flags_cancelar = svc_alarma_codificar(false, 0, 0);
		svc_alarma_activar(flags_cancelar, ev_LEDS_GUITAR_HERO, 0);
	
		// Cancelar suscripciones
		svc_GE_cancelar(ev_LEDS_GUITAR_HERO, evento_leds_guitar_hero);
    svc_GE_cancelar(ev_FIN_GUITAR_HERO, fin_partida_guitar_hero);
		svc_GE_cancelar(ev_BOTONES_GUITAR_HERO, evento_boton_guitar_hero);
		svc_GE_cancelar(ev_TIMEOUT_LED, evento_timeout_guitar_hero);

    // Finalizar características de partída 
    en_partida = 0;
    num_partidas ++;
    puntuacion_total += puntuacion;
		notas_tocadas = 0;

    // Información de las características principales al usuario
    estadisticas_guitar_hero(); 
		aciertos = 0;
		fallos = 0;
		puntuacion = 0;

    // Reiniciamos compás
    estados_notas[0] = 0;
		estados_notas[1] = 0;
		estados_notas[2] = 0;	

    // Periodo de los leds disminuye
    if(periodo_leds >= T_RESTA_PARTIDA * 2){
       periodo_leds -= T_RESTA_PARTIDA;
    }
    partida_guitar_hero();
}

//------------------------------ MAIN ------------------------------//
void partida_guitar_hero() {
		main_secuencias_gh(0);

    // Suscripciones
    svc_GE_suscribir(ev_FIN_GUITAR_HERO, 0, fin_partida_guitar_hero);
		svc_GE_suscribir(ev_BOTONES_GUITAR_HERO, 0, evento_boton_guitar_hero);
		svc_GE_suscribir(ev_TIMEOUT_LED, 0, evento_timeout_guitar_hero);
		svc_GE_suscribir(ev_LEDS_GUITAR_HERO, 0, evento_leds_guitar_hero);
}

void guitar_hero(unsigned int num_leds){
    leds = num_leds;
    periodo_leds = PERIODO_LEDS;

    //Poner en marcha lo necesario del background
    drv_monitor_iniciar();
    drv_consumo_iniciar((MONITOR_id_t)MONITOR_CONSUMO);
    rt_FIFO_inicializar((MONITOR_id_t)MONITOR_FIFO);  
    rt_GE_iniciar((MONITOR_id_t)MONITOR_GE);
		random_iniciar(drv_tiempo_actual_us());

		drv_wdt_iniciar(PERIODO_WDT);

    drv_botones_iniciar(manejador_interrupcion_botones_gh);
		
		drv_uart_init(9600);

    partida_guitar_hero();
	
		rt_GE_lanzador();
}
