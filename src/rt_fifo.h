/* *****************************************************************************
 * P.H.2025: Runtime para la implementación de colas de tipo FIFO
 * Colas de tamaño limitado almacenan los distintos eventos a ejecutar e información
 * adicionar relativa a estos
 */

#ifndef RT_FIFO_H
#define RT_FIFO_H

#include <stdint.h>
#include "rt_evento_t.h"
#include "drv_tiempo.h"
#include "drv_monitor.h"

#define RT_FIFO_TAMANO 128


/* Estructura de un evento almacenado en la cola */
typedef struct {
    EVENTO_T ID_EVENTO;  // usa el enum del profesor
    uint32_t auxData;
    Tiempo_us_t TS;      // timestamp en microsegundos
} EVENTO;

/**
 * Inicializa la cola de eventos.
 */
void rt_FIFO_inicializar(uint32_t monitor);

/**
 * Encola un nuevo evento en la cola.
 */
void rt_FIFO_encolar(EVENTO_T ID_evento, uint32_t auxData);

/**
 * Extrae el siguiente evento pendiente (FIFO).
 */
uint8_t rt_FIFO_extraer(EVENTO_T *ID_evento, uint32_t *auxData, Tiempo_us_t *TS);

/**
 * Devuelve el número de veces que se ha encolado un tipo de evento.
 */
uint32_t rt_FIFO_estadisticas(EVENTO_T ID_evento);

#endif /* RT_FIFO_H */
