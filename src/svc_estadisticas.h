/* ***************************************************************************************
 * P.H.2025: Interfaz del Servicio de Estadísticas
 * Proporciona la interfaz necesaria para recolectar, calcular y almacenar
 * estadísticas de rendimiento del sistema en tiempo real.
 *
 * Funciones:
 * - Inicializar el servicio de estadísticas
 * - Registrar timestamps de eventos del sistema
 * - Calcular tiempos de respuesta de IRQs y usuario
 * - Medir tiempos de permanencia en colas FIFO
 * - Calcular estadísticas (media, máximo) de eventos
 * 
 * Nota:
 *  Las estadísticas se almacenan en una estructura global y se calculan de forma
 *  incremental para minimizar el uso de memoria y procesamiento.
 *  Los tiempos se miden en microsegundos (US) y milisegundos (MS) según el caso.
 *
 * Autores:
 * - Pablo Plumed
 * - Alex Asensio
 *************************************************************************************** */

#ifndef SVC_ESTADISTICAS_H
#define SVC_ESTADISTICAS_H

#include <stdint.h>
#include "rt_evento_t.h"

// Tipos de eventos para medición de tiempos
typedef enum {
    e_LANZA_IRQ,                  // Marca el lanzamiento de una interrupción
    e_ATIENDE_IRQ,                // Marca la atención de una interrupción
    e_TERMINA_SECUENCIA,          // Marca el fin de una secuencia del sistema
    e_EMPIEZA_PULSAR,             // Marca el inicio de pulsación del usuario
    e_TIEMPO_INICIO_DESPIERTO,    // Marca entrada en estado activo
    e_TIEMPO_FIN_DESPIERTO,       // Marca salida de estado activo
    e_TIEMPO_INICIO_ESPERA,       // Marca entrada en estado de espera
    e_TIEMPO_FIN_ESPERA,          // Marca salida de estado de espera
    e_TIEMPO_ENCOLAR,             // Marca encolamiento de evento en FIFO
    e_TIEMPO_DESENCOLAR           // Marca desencolamiento de evento en FIFO
} estadisticas_evento_t;

//Estadísticas de un evento en cola FIFO
typedef struct {
    uint64_t tiempo_encolar;        // Timestamp de encolamiento (MS)
    uint64_t tiempo_desencolar;     // Timestamp de desencolamiento (MS)
    int64_t tiempo_fifo_max_MS;     // Tiempo máximo en cola (MS)
    int64_t tiempo_fifo_med_MS;     // Tiempo medio en cola (MS)
    uint32_t mediciones;            // Número de mediciones realizadas
    uint8_t flag_encolar;           // Flag de estado (1=listo para encolar, 0=encolado)
} evento_fifo_t;

// Estructura principal de estadísticas del sistema
typedef struct {
    // Estadísticas de interrupciones (IRQ)
    uint64_t lanza_irq;                     // Timestamp de lanzamiento de IRQ (US)
    uint64_t atiende_irq;                   // Timestamp de atención de IRQ (US)
    uint64_t tiempo_respuesta_irq_US;       // Tiempo de respuesta de IRQ (US)
    
    // Estadísticas de respuesta del usuario
    uint64_t termina_secuencia;             // Timestamp de fin de secuencia (US)
    uint64_t empieza_pulsar;                // Timestamp de inicio de pulsación (US)
    uint64_t tiempo_respuesta_usuario_US;   // Tiempo de respuesta del usuario (US)
    
    // Estadísticas de estados del sistema
    uint64_t tiempo_inicio_despierto;       // Timestamp de inicio estado activo (US)
    uint64_t tiempo_fin_despierto;          // Timestamp de fin estado activo (US)
    uint64_t tiempo_despierto_US;           // Tiempo total en estado activo (US)
    
    uint64_t tiempo_inicio_espera;          // Timestamp de inicio estado espera (US)
    uint64_t tiempo_fin_espera;             // Timestamp de fin estado espera (US)
    uint64_t tiempo_espera_US;              // Tiempo total en estado espera (US)
    
    // Estadísticas de eventos en cola FIFO
    evento_fifo_t eventos[EVENT_TYPES];     // Array de estadísticas por tipo de evento
} estadisticas_t;

extern estadisticas_t estadisticas;

// Pre: Ninguna
// Post: Todos los campos de la estructura estadisticas están inicializados a 0
//       Los flags de encolar están en 1 (listos para encolar)
void svc_estadisticas_iniciar(void);

// Pre: svc_estadisticas_iniciar() debe haberse llamado previamente
//      drv_tiempo debe estar inicializado y funcionando correctamente
//      El parámetro evento debe ser un valor válido de estadisticas_evento_t
// Post: Se registra el timestamp del evento especificado
//       Si el evento completa un par (inicio-fin), se calcula el tiempo transcurrido
//       Los timestamps temporales se limpian después de calcular tiempos
void svc_estadisticas_set_tmp(estadisticas_evento_t evento);

// Pre: svc_estadisticas_iniciar() debe haberse llamado previamente
//      drv_tiempo debe estar inicializado y funcionando correctamente
//      El parámetro evento debe ser un valor válido de EVENTO_T
//      Para desencolar, debe haberse llamado previamente con e_TIEMPO_ENCOLAR
// Post: Si es encolamiento: se registra el timestamp y se marca como encolado
//       Si es desencolamiento: se calcula el tiempo en cola, se actualiza máximo,
//       se calcula la media incremental y se incrementa el contador de mediciones
void svc_estadisticas_set_tmp_fifo(EVENTO_T evento, 
                                    estadisticas_evento_t desencolar_encolar);

#endif  //SVC_ESTADISTICAS_H
