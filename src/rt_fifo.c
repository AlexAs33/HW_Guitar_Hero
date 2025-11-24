/* *****************************************************************************
 * P.H.2025: Runtime para la implementación de colas de tipo FIFO
 * Colas de tamaño limitado almacenan los distintos eventos a ejecutar e información
 * adicionar relativa a estos
 */
 
#include "rt_fifo.h"
#include <stdio.h>
#include "drv_leds.h"

static struct {
    EVENTO buffer[RT_FIFO_TAMANO];
    uint8_t siguiente_a_tratar;
    uint8_t siguiente_libre;
    uint32_t monitor_overflow;
} fifo;

static uint32_t estadisticas[EVENT_TYPES];

/* Inicializa la cola y el monitor de overflow */
void rt_FIFO_inicializar(MONITOR_id_t monitor)
{
    fifo.siguiente_a_tratar = 0;
    fifo.siguiente_libre    = 0;
    fifo.monitor_overflow   = monitor;

    for (int i = 0; i < RT_FIFO_TAMANO; ++i) {
        fifo.buffer[i].ID_EVENTO = ev_VOID;
        fifo.buffer[i].auxData   = 0;
        fifo.buffer[i].TS        = 0;
    }

    for (int i = 0; i < EVENT_TYPES; ++i) {
        estadisticas[i] = 0;
    }
}

/* Añade un evento a la cola */
void rt_FIFO_encolar(EVENTO_T ID_evento, uint32_t auxData)
{
    uint8_t siguiente = (fifo.siguiente_libre + 1) % RT_FIFO_TAMANO;

    /* Overflow: la cola se llenó */
    if (siguiente == fifo.siguiente_a_tratar)
    {
      drv_monitor_marcar(fifo.monitor_overflow);
			for(LED_id_t id = 1; id <= LEDS_NUMBER; id++){
				drv_led_establecer(id, LED_ON);
			}
      while(1);  // bucle infinito 
    }

    EVENTO *evt    = &fifo.buffer[fifo.siguiente_libre];
    evt->ID_EVENTO = ID_evento;
    evt->auxData   = auxData;
    evt->TS        = drv_tiempo_actual_us();

    fifo.siguiente_libre = siguiente;

    /* Estadísticas */
    if ((uint32_t)ID_evento < EVENT_TYPES) {
        estadisticas[ID_evento]++;
    }
}

/* Extrae el siguiente evento pendiente, devuelve 0 si no hay */
// En rt_fifo.c
uint8_t rt_FIFO_extraer(EVENTO_T *ID_evento, uint32_t *auxData, Tiempo_us_t *TS)
{
    if (fifo.siguiente_a_tratar == fifo.siguiente_libre) 
        return 0;  // No había eventos

    EVENTO *evt = &fifo.buffer[fifo.siguiente_a_tratar];
    *ID_evento = evt->ID_EVENTO;
    *auxData   = evt->auxData;
    *TS        = evt->TS;

    fifo.siguiente_a_tratar = (fifo.siguiente_a_tratar + 1) % RT_FIFO_TAMANO;

    return 1;  // Devuelve 1 si extrajo algo (independiente de cuántos quedan)
}

/* Devuelve cuántas veces se encoló un evento */
uint32_t rt_FIFO_estadisticas(EVENTO_T ID_evento)
{
    if ((uint32_t)ID_evento < EVENT_TYPES)
        return estadisticas[ID_evento];
    return 0;
}
