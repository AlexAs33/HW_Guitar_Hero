/* *****************************************************************************
 * P.H.2025: main.c
 *
 * Programa principal de la pr�ctica 2 de Proyecto Hardware.
 *
 * - Inicializa la HAL de GPIO y el driver de LEDs.
 * - Invoca a las funciones de parpadeo (blink_v1 o blink_v2) seg�n la sesi�n.
 *
 * Versi�n de pr�cticas:
 *   - Sesi�n 1: blink_v1 -> parpadeo con retardo por bucle de instrucciones.
 *   - Sesi�n 2: blink_v2 -> parpadeo controlado por temporizador.
 *
 * Notas:
 *   - El identificador de LED comienza en 1 (primer LED definido en board.h).
 *   - El bucle principal no retorna (sistema embebido).
 *
 * Autores: Enrique Torrs, Alex Asensio, Pablo Plumed
 * Universidad de Zaragoza
 * ****************************************************************************/

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include "svc_alarma.h"

#include "rt_fifo.h"
#include "rt_evento_t.h"
#include "rt_GE.h"

#include "rt_fifo.h"
#include "rt_evento_t.h"
#include "rt_GE.h"

#include "drv_leds.h"
#include "drv_tiempo.h"
#include "drv_consumo.h"
#include "drv_monitor.h"
#include "drv_botones.h"
#include "drv_sc.h"
#include "drv_wdt.h"
#include "drv_uart.h"

#include "hal_gpio.h"

#include "board.h"
#include "random.h"

#include "app_guitar_hero.h"
#include "app_bit_counter_strike.h"

#ifdef DEBUG 
	#include "svc_estadisticas.h"
#endif

#define RETARDO_MS 500u

#define TEST_WATCHDOG  1
#define TEST_BOTONES   2
#define TEST_OVERFLOW  3
#define TEST_UART      4
#define TEST_PARTITURA 5

#define TEST  TEST_UART

#define TESTS   0
#define BCS     6
#define GH		  7
#define VERSION GH

#define MONITOR_CONSUMO 1
#define MONITOR_FIFO 2
#define MONITOR_GE 3

/* Prototipos */
void blink_v1(LED_id_t id);
void blink_v2(LED_id_t id);
void blink_v3(LED_id_t id);
void blink_v4(LED_id_t id);
void blink_v3_bis(LED_id_t id);

void svc_alarma_actualizar(EVENTO_T evento, uint32_t aux);

void blink_v3_bis(LED_id_t id);

void svc_alarma_actualizar(EVENTO_T evento, uint32_t aux);


/* *****************************************************************************
 * BLINK v1: parpadeo de un LED conmutando on/off.
 * Retardo por bucle de instrucciones (busy-wait). Solo usa el driver de LEDs.
 * para realizar la primera sesi�n de la practica
 */
void blink_v1(LED_id_t id) {

	  drv_led_establecer(id, LED_ON);
    while (1) {
        volatile uint32_t tmo = 10000000u;  /* ajustad para vuestro reloj */
        while (tmo--) { /* nop */ }
        (void)drv_led_conmutar(id);
    }
}

/* *****************************************************************************
 * BLINK v2, parpadeo de un led conmutando on/off 
 * activacion por tiempo, usa tanto manejador del led como el del tiempo
 * para realizar en la segunda sesion de la practica, version a entregar
 */
void blink_v2(LED_id_t id) {
    Tiempo_ms_t siguiente_activacion;	

    /* Encender LED inicial */
    drv_led_establecer(id, LED_ON);

    /* Tomar tiempo de referencia */
    siguiente_activacion = drv_tiempo_actual_ms();
	
    /* Bucle principal: conmutar con temporizador */
    while (true) {
        siguiente_activacion += RETARDO_MS;     /* calcular la siguiente activaci�n */
			
        drv_tiempo_esperar_hasta_ms(siguiente_activacion);
        (void)drv_led_conmutar(id);

        /* Aqu� podr�an a�adirse otras tareas peri�dicas */
    }
}

// Funcion de callback
void leds_c(LED_id_t id) {
	drv_monitor_desmarcar((MONITOR_id_t)3);
	drv_led_conmutar(id+1);
	drv_monitor_marcar((MONITOR_id_t)3);
}

/* *********************************************************************************
 * BLINK v3: Parpadeo de un led con CPU dormida a base de interrupciones
 */
void blink_v3(LED_id_t id){
		drv_consumo_iniciar((MONITOR_id_t)3);
		
		/* Encender LED inicial */
		drv_led_establecer(id, LED_ON);
		
		drv_tiempo_periodico_ms(RETARDO_MS, leds_c, id);
		while(true)	drv_consumo_esperar();
}

/* *********************************************************************************
 * BLINK v4: Parpadeo de un led con cola de eventos y monitorización de la ejecución
 */
void blink_v4(LED_id_t id)
{
    EVENTO_T EV_ID_evento;
    uint32_t EV_auxData;
    Tiempo_us_t EV_TS;
	
    // Inicializar consuma, monitores y cola de eventos
		drv_consumo_iniciar((MONITOR_id_t)3);
    drv_monitor_iniciar();
    rt_FIFO_inicializar((MONITOR_id_t)id);

    // Encender el LED al inicio
    drv_led_establecer(id, LED_ON);

    // Configurar temporizador periódico de 500 ms
    drv_tiempo_periodico_ms(RETARDO_MS, rt_FIFO_encolar, ev_T_PERIODICO);

    // Bucle principal orientado a eventos
    while (1)
    {
        if (rt_FIFO_extraer(&EV_ID_evento, &EV_auxData, &EV_TS)) {
            // Procesar solo evento periódico
            if (EV_ID_evento == ev_T_PERIODICO)
                drv_led_conmutar(id);
		}
				// No hay eventos pendientes
        else
            drv_consumo_esperar();
    }
}

void drv_boton_estado_actualizar(EVENTO_T evento, uint32_t auxData) {}

void blink_v3_bis(LED_id_t id) 
{
  drv_monitor_iniciar();

	drv_consumo_iniciar((MONITOR_id_t)3);
		
	drv_botones_iniciar(drv_boton_estado_actualizar, ev_VOID);
		
	drv_tiempo_periodico_ms(RETARDO_MS, leds_c, id);
	
	int palpitaciones = 0;
    while(true) {
        if (palpitaciones < 20) {
            drv_consumo_esperar();
            palpitaciones++;
        }
        else {
            drv_consumo_dormir();
            palpitaciones = 0;
        }
    }
}

#ifdef DEBUG
/* *********************************************************************************
 * TESTS
 */

void test_overflow(){
		drv_monitor_iniciar();
		rt_FIFO_inicializar((MONITOR_id_t)3);
		for (int i = 0; i < RT_FIFO_TAMANO; i++) 
				rt_FIFO_encolar(ev_VOID, 0);
}

void test_watchdog(LED_id_t id) {
	  drv_wdt_iniciar(5);
		rt_GE_iniciar((MONITOR_id_t)1);
		test_overflow();
		while (1);
}

void boton_pulsado(int32_t id_boton) {
    if (id_boton != -1) {
        drv_led_conmutar((LED_id_t)1);
		drv_sc_disable();
    // Llama a la lógica del juego directamente con el botón pulsado
		drv_botones_actualizar(ev_PULSAR_BOTON, id_boton);
		drv_sc_enable();
    }
}

void test_botones() {
    // Inicializar consuma, monitores y cola de eventos
    drv_consumo_iniciar((MONITOR_id_t)2);
    drv_monitor_iniciar();
	
		rt_GE_iniciar((MONITOR_id_t) 1);
    rt_FIFO_inicializar((MONITOR_id_t)3);

		drv_botones_iniciar(boton_pulsado, ev_VOID);

    rt_GE_lanzador();
}

void test_uart() {
    drv_uart_init(9600);
    drv_uart_puts("\r\nUART TEST INICIADO\r\n");
		drv_uart_puts("\r\nESTOY EN MODO DEBUG\r\n\n");
	
    int contador = 0;

    while (1) {
        // mensaje de info
        char buf[32];
        sprintf(buf, "Contador: %d", contador++);
        UART_LOG_INFO(buf);

        // delay cutre
        for (volatile int i = 0; i < 500000; i++);
    }
}

void crear_partitura_aleatoria() {
    drv_uart_init(9600);
    random_iniciar(drv_tiempo_actual_us());

    uint8_t partitura[TAM_PARTITURA] = {};
		for(int j = 1; j < 6; j++) 
		{
			char msg[64];
			sprintf(msg, "PARTITURA NUMERO: %d\r\n===========================", j);
		  UART_LOG_INFO(msg);
			for (int i = 0; i < TAM_PARTITURA; i++) {
					partitura[i] = random_value(0, 3);   // notas entre 0 y 3

					char msg[8];
					sprintf(msg, "%c%c ",
							(partitura[i] & 0x02) ? '1' : '0',   // bit 1
							(partitura[i] & 0x01) ? '1' : '0');  // bit 0

					drv_uart_puts(msg);

					if ((i + 1) % 10 == 0)
							drv_uart_puts("\r\n");
			}
			drv_uart_puts("\r\n");
		}
}
#endif
/* *****************************************************************************
 * MAIN, Programa princ * para la primera sesion se debe usar la funcion de blink_v1 sin temporizadores
 * para la entrega final se debe incocar blink_v2
 */




int main(void){
	
#ifdef DEBUG 
				svc_estadisticas_set_tmp(e_TIEMPO_INICIO_DESPIERTO);
#endif

	
	uint32_t Num_Leds;
	volatile uint8_t timer_iniciado;

    if (!drv_tiempo_iniciar()) return 1;  // Inicializa el driver de tiempo

	hal_gpio_iniciar();	// llamamos a iniciar gpio antesde que lo hagan los drivers
	
	/* Configurar LEDs (IDs v�lidos: 1..num_leds) */
	Num_Leds = drv_leds_iniciar();	
	
	if (Num_Leds > 0){
#if   VERSION == 1
			/* Sesi�n 1: parpadeo por lazo ocupado */
			blink_v1((LED_id_t)1);
		
#elif VERSION == 2
			timer_iniciado = drv_tiempo_iniciar();
			/* Sesi�n 2: temporizador */
			if (timer_iniciado)
				blink_v2((LED_id_t)2);
			
#elif VERSION == 3
			blink_v3((LED_id_t)3);

#elif VERSION == 4
			blink_v4((LED_id_t)4);

#elif VERSION == 5
      blink_v3_bis((LED_id_t)3);

#elif VERSION == BCS
        bit_counter_strike(Num_Leds);

#elif VERSION == GH
				// Inicialización de módulos necesarios
				drv_monitor_iniciar();
				drv_consumo_iniciar((MONITOR_id_t)MONITOR_CONSUMO);
				rt_FIFO_inicializar((MONITOR_id_t)MONITOR_FIFO);  
				rt_GE_iniciar((MONITOR_id_t)MONITOR_GE);
				random_iniciar(drv_tiempo_actual_us());

				drv_wdt_iniciar(PERIODO_WDT);

				drv_botones_iniciar(manejador_botones_guitar_hero, ev_FIN_GUITAR_HERO);

				drv_uart_init(9600);
				app_guitar_hero_iniciar(Num_Leds);
		
#else
	#ifdef DEBUG
			#if TEST == TEST_BOTONES
					test_botones();
					
			#elif TEST == TEST_WATCHDOG
					test_watchdog((LED_id_t)1);
					
			#elif TEST == TEST_OVERFLOW
					test_overflow();

					#elif TEST == TEST_UART
									test_uart();

					#elif TEST == TEST_PARTITURA
									crear_partitura_aleatoria();
			#endif
	#endif
#endif
		}	

    /* En sistemas empotrados normalmente no se retorna */
    while (1) { /* idle */ }
    /* return 0; */
}
