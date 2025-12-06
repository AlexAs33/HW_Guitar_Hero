/* ***************************************************************************************
 * P.H.2025: Interfaz del Runtime de las colas tipo FIFO
 * Proporciona la interfaz necesaria para inicializar, encolar, extraer y consultar 
 *  eventos en este tipo de colas.
 *
 * Funciones:
 * - Inicializar la cola con un monitor de overflow asociado
 * - Encolar eventos con un parametro
 * - Extraer eventos con un parámetro y tiempo asociados
 * - Consultar el número de eventos encolados de un tipo
 * 
 * Nota:
 *  por las características del hardware donde se trabaja el número de eventos encolados está limitado.
 *  Si se excede el programa se detendrá en un bucle infinito.
 *
 * Autores:
 * - Pablo Plumed
 * - Alex Asensio
 *************************************************************************************** */

#ifndef RT_FIFO_H
#define RT_FIFO_H

#include <stdint.h>
#include "rt_evento_t.h"
#include "drv_tiempo.h"
#include "drv_monitor.h"

#define RT_FIFO_TAMANO 64


/* Estructura de un evento almacenado en la cola */
typedef struct {
    EVENTO_T ID_EVENTO;     //enum de eventos
    uint32_t auxData;       //parámetro asociado
    Tiempo_us_t TS;         //timestamp en microsegundos
} EVENTO;

//Pre:  El identificador de monitor pasado es válido.
//Post: La cola queda vacía; los índices se reinician a 0, el monitor de overflow queda asociado,
//      el buffer queda inicializado con eventos vacíos y las estadísticas se ponen a cero.
void rt_FIFO_inicializar(uint32_t monitor);

//Pre:  La cola no está llena; ID_evento es un valor válido del enum EVENTO_T.
//Post: El evento queda almacenado en la siguiente posición libre, con su auxData y timestamp.
//      El puntero de escritura avanza de forma circular.
//      Si la cola se llena, se marca el monitor de overflow, se encienden todos los LEDs
//      y el programa se detiene en un bucle infinito.
void rt_FIFO_encolar(EVENTO_T ID_evento, uint32_t auxData);

//Pre:  Los punteros de lectura y escritura reflejan un estado válido de la cola.
//Post: Si la cola no está vacía, se copia en los parámetros de salida el ID del evento,
//      su auxData y su timestamp, y el puntero de lectura avanza circularmente.
//      Devuelve 1 si se ha extraído un evento; devuelve 0 si la cola estaba vacía
//      (sin modificar parámetros de salida).
uint8_t rt_FIFO_extraer(EVENTO_T *ID_evento, uint32_t *auxData, Tiempo_us_t *TS);

//Pre:  ID_evento es un valor válido del enum EVENTO_T.
//Post: Devuelve el número de veces que dicho evento ha sido encolado. Si el ID no es válido,
//      devuelve 0 y no modifica estado interno.
uint32_t rt_FIFO_estadisticas(EVENTO_T ID_evento);

#endif /* RT_FIFO_H */
