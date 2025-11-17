/* ***************************************************************************************
 * P.H.2025: Interfaz del Juego Bit Counter Strike
 * Este archivo define la interfaz del juego Bit Counter Strike.
 * Este juego consiste en pulsar el bot�n asociado aun led. 
 * Si se falla se pierde el juego, se marcar� con todos los leds encendidos.
 * Tambi�n tiene asociado un tiempo de inactividad que apagar� la placa.
 *
 * Funciones:
 *  - Iniciar el juego.
 *
 * Autores:
 * - Pablo Plumed
 * - Alex Asensio
 *************************************************************************************** */

#ifndef BIT_COUNTER_STRIKE_H
#define BIT_COUNTER_STRIKE_H

//Variables Juego
#define PERIODO_BIT_COUNTER 500u	//ms encendido leds
#define PERIODO_WD          10		//segundos reinicio WDT
#define TIMEOUT_LED_MS 	    2000u //ms para clicar led
#define INICIAR_BCS         10u

//Pre: --
//Funci�n que inicia el juego.
void bit_counter_strike(unsigned int num_leds);

#endif	//BIT_COUNTER_STRIKE_H
