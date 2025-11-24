/* *****************************************************************************
 * P.H.2025: Runtime para la implementación de distintos eventos 
 */
 
#ifndef RT_EVENTO_T_H
#define RT_EVENTO_T_H

#include <stdint.h>
#include "drv_tiempo.h" 

#define ev_NUM_EV_USUARIO 1
#define ev_USUARIO  {ev_ACT_INACTIVIDAD}

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
		ev_LEDS_GUITAR_HERO = 10,
    ev_FIN_GUITAR_HERO = 11,
    ev_SEC_INI_FIN = 12,
		ev_ACT_INACTIVIDAD = 13
} EVENTO_T;  // mapea a uint32_t

#define EVENT_TYPES 14

#endif /* RT_EVENTO_T_H */
