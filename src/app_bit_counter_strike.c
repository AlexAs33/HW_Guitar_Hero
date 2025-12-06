/* ***************************************************************************************
 * P.H.2025: Interfaz del Juego Bit Counter Strike
 * Este archivo define la interfaz del juego Bit Counter Strike.
 * Este juego consiste en pulsar el botón asociado aun led. 
 * Si se falla se pierde el juego, se marcará con todos los leds encendidos.
 * También tiene asociado un tiempo de inactividad que apagará la placa.
 *
 * Funciones:
 *  - Iniciar el juego.
 *
 * Autores:
 * - Pablo Plumed
 * - Alex Asensio
 *************************************************************************************** */

#include "app_bit_counter_strike.h"

#include "rt_fifo.h"
#include "rt_GE.h"

#include "svc_GE.h"
#include "svc_alarma.h"

#include "drv_botones.h"
#include "drv_consumo.h"
#include "drv_leds.h"
#include "drv_wdt.h"

#include "svc_random.h"

#include <stdio.h>
#include <stdlib.h>

//Monitores Overflow
#define MONITOR_CONSUMO 1
#define MONITOR_FIFO 2
#define MONITOR_GE 3

//Variables globales para el juego
static unsigned int numLeds = 0;	//Número de leds
static LED_id_t led = 0;   				//Identificador del LED encendido (0 significa apagado)
static uint32_t boton = 0; 				//Identificador del último botón pulsado (0 significa ninguno)
bool ledApagado = true;						//true si led apagado.

//Manejador de interrupciones de bajo nivel para los botones.
//Asocia el botón pulsado al evento del juego para tratarlo según la lógica
void manejador_interrupcion_botones(int32_t id_pin, int32_t id_boton) {
    if (id_boton != -1) {
        drv_botones_actualizar(ev_PULSAR_BOTON, id_pin);		//Actulaiza el estado a bajo nivel del pin
        rt_FIFO_encolar(ev_LEDS_BIT_COUNTER_STRIKE, id_boton + 1);	//Produce un evento del juego asociado al botón.
    }
}

//Pasa al siguiente estado del juego en base al estado y transición anteriores (botón pulsado)
void actualizar_bit_counter_strike(EVENTO_T evento, uint32_t auxData) {
  (void)evento;
  
  // Si auxData no es 0, el evento viene de una pulsación de botón
  if (auxData != 0) 
      boton = auxData;
  
  if (ledApagado) {	//Si no hay ningún LED encendido, se selecciona uno ALEATORIO
										//Ignoramos pulsaciones de botón si el juego no ha empezado un turno.
    if (auxData != 0) return;
    
    led = svc_random_value((uint32_t)1, numLeds);
    
    drv_led_establecer(led, LED_ON);
    boton = 0; //Reinicia el registro del último botón pulsado
    ledApagado = false;
		//Activar alarma para pulsar led
		uint32_t flags_timeout = svc_alarma_codificar(false, TIMEOUT_LED_MS, 0);
		svc_alarma_activar(flags_timeout, ev_TIMEOUT_LED, led);
  } 
  else {	//Si hay un LED encendido, verifica si el botón pulsado es el correcto
    if (boton != led) {	//Fallo del jugador. GAME OVER
			
      for (LED_id_t i = 1; i <= numLeds; i++) 	//Enciende todos los leds para indicar que ha perdido
        drv_led_establecer(i, LED_ON);

      led = 0;
      boton = 0;
			//cancelar alarma tiempo boton
			uint32_t flags_cancelar = svc_alarma_codificar(false, 0, 0);
			svc_alarma_activar(flags_cancelar, ev_TIMEOUT_LED, led);

      while(1);    // Bucle infinito (reset cuando llegue el watchdog
    } 
    else {	// El jugador ha acertado
      drv_led_establecer(led, LED_OFF); //Apaga el LED actual para preparar el siguiente
      ledApagado = true;
      //cancelar alarma tiempo boton
			uint32_t flags_cancelar = svc_alarma_codificar(false, 0, 0);
			svc_alarma_activar(flags_cancelar, ev_TIMEOUT_LED, led);
			
      svc_alarma_activar(0, ev_LEDS_BIT_COUNTER_STRIKE, 0);	//Cancela la alarma de timeout anterior
      svc_alarma_activar(PERIODO_BIT_COUNTER, ev_LEDS_BIT_COUNTER_STRIKE, 0);	// Activa el próximo paso del juego
    }
  }
}

//Función que asegura que se pulsa el botón antes del tiempo determinado
void timeout_led(EVENTO_T evento, uint32_t auxData) {
  (void)evento;
  if (auxData == led && !ledApagado) {
    for (LED_id_t i = 1; i <= numLeds; i++) {
      drv_led_establecer(i, LED_ON);
    }
    led = 0;
    boton = 0;
    while(1);
  }
}

void iniciarBCS() {
	for(int i = 1; i <= numLeds; i++) {
		drv_led_establecer(i, LED_ON);
		for (int j = 0; j <= INICIAR_BCS; j++)
			drv_consumo_esperar();
	}
	
	for(int i = 1; i <= numLeds; i++)
		drv_led_establecer(i, LED_OFF);
}

//Función principal del juego
//Pre: num_leds <= numero de leds del hardware. Leds y gpio iniciados
//Inicializa todo lo necesario
void bit_counter_strike(unsigned int num_leds) {
    numLeds = num_leds;
	
		//Iniciar runtime y drivers
    drv_monitor_iniciar();
    drv_consumo_iniciar((MONITOR_id_t)MONITOR_CONSUMO);
    rt_FIFO_inicializar((MONITOR_id_t)MONITOR_FIFO);  
    rt_GE_iniciar((MONITOR_id_t)MONITOR_GE);
	
    drv_botones_iniciar(manejador_interrupcion_botones, ev_PULSAR_BOTON, ev_VOID);	//Asociar botones a lógica local
	
		uint32_t alarmaBCS = svc_alarma_codificar(true, PERIODO_BIT_COUNTER, 0);
    svc_alarma_activar(alarmaBCS, ev_LEDS_BIT_COUNTER_STRIKE, 0);	//Encender led cada PERIODO_BIT_COUNTER
    svc_GE_suscribir(ev_LEDS_BIT_COUNTER_STRIKE, 0, actualizar_bit_counter_strike);	//Atender a eventos del juego
		svc_GE_suscribir(ev_TIMEOUT_LED, 0, timeout_led);	//evento para pulsar boton antes de x tiempo
	
    svc_random_iniciar(drv_tiempo_actual_us());
    drv_wdt_iniciar(PERIODO_WD);
	
	  iniciarBCS();
	
    rt_GE_lanzador();	//Gestiona todos los eventos del juego
}
