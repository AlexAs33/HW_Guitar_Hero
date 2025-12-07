/* ***************************************************************************************
 * P.H.2025: Interfaz del Runtime que almacena los tipos de eventos con un id asociado.
 *
 * Autores:
 * - Pablo Plumed
 * - Alex Asensio
 *************************************************************************************** */
 
#ifndef RT_EVENTO_T_H
#define RT_EVENTO_T_H

#include <stdint.h>
#include "drv_tiempo.h" 

#define ev_NUM_EV_USUARIO 1
#define ev_USUARIO  {ev_PULSAR_BOTON}

/* Enum de eventos para encolar */
typedef enum {
    ev_VOID = 0,
    ev_T_PERIODICO = 1,
    ev_PULSAR_BOTON = 2,
    ev_BOTON_DEBOUNCE   = 3,
    ev_BOTON_DEBOUNCE_2 = 4,
    ev_BOTON_DEBOUNCE_3 = 5,
    ev_BOTON_DEBOUNCE_4 = 6,
    ev_INACTIVIDAD = 7,
		ev_LEDS_BIT_COUNTER_STRIKE = 8,
		ev_TIMEOUT_LED	= 9,
    ev_FIN_GUITAR_HERO = 10,
    ev_GUITAR_HERO = 11,
} EVENTO_T;  // mapea a uint32_t

#define EVENT_TYPES 13

#endif /* RT_EVENTO_T_H */
