/* ***************************************************************************************
 * P.H.2025: Interfaz del Servicio de Alarmas
 * Proporciona la interfaz necesaria para iniciar, codificar, activar y actualizar
 * Alarmas que generarán eventos
 *
 * Funciones:
 * - Iniciar el servicio con un monitor de overflow y una función de inactividad asociada
 * - Codificar alarmas con un tiempo específico
 * - Activar alarmas con un veneto asociado
 * - Actualizar alarmas
 * 
 * Nota:
 *  por las características del hardware donde se trabaja el número de alarmas está limitado.
 *  Si se excede el programa se detendrá en un bucle infinito.
 *
 * Autores:
 * - Pablo Plumed
 * - Alex Asensio
 *************************************************************************************** */

#ifndef SVC_ALARMAS_H
#define SVC_ALARMAS_H

#include <stdint.h>
#include <stdbool.h>
#include "rt_evento_t.h"
#include "rt_fifo.h"	
#include "drv_monitor.h"
#include "drv_tiempo.h"
#include "svc_GE.h"

//Variables de las alarmas
#define svc_ALARMAS_MAX 4						//Máximas alarmas disponibles
#define svc_ALARMAS_PERIODO_MS 10

//Estructura de las alarmas
typedef struct {
    bool activa;
    bool periodica;
    Tiempo_ms_t retardo_ms;
    Tiempo_ms_t contador_ms;
    EVENTO_T evento;
    uint32_t auxData;
} svc_alarma_t;

//Pre: monitor que registrará el overflow, función a la que llama el evento periódico, evento periodico
//Post: Inicializa el servicio de alarmas.
//			Si se supera el número de alarmas --> overflow
//			cuendo se da un evento periódico (x veces) se llama a una función de callback --> evento inactividad
void svc_alarma_iniciar(MONITOR_id_t M_overflow, void (*cb_a_llamar)(EVENTO_T, uint32_t), EVENTO_T ev_periodico);

//Pre: flags coherentes
//Post: devuelve el resultado de la codificación de la alarma en base a los parámetros
uint32_t svc_alarma_codificar(bool periodico, Tiempo_ms_t retardo_ms, uint8_t flags);

//Pre: flags coherentes, evento existente
//Post: activa una alarma asociada a un evento o la reprograma si ya existe
void svc_alarma_activar(uint32_t alarma_flags, EVENTO_T ID_EVENTO, uint32_t aux_DATA);

//Pre: evento existente
//Post: actualiza la alarma asociada a un evento
void svc_alarma_actualizar(EVENTO_T evento, uint32_t aux);

#endif
