/* ***************************************************************************************
 * P.H.2025: Interfaz del Driver de botones
 * Proporciona la interfaz necesaria para gestionar los botones con tratamiento
 * de rebotes mediante MSF, interrupciones externas y alarmas periódicas.
 *
 * Funciones:
 * - Inicialización del driver de botones
 * - Actualización del estado interno según eventos recibidos
 * - Gestión de estados individuales de cada botón
 *
 * Autores:
 * - Pablo Plumed
 * - Alex Asensio
 *************************************************************************************** */

#ifndef DRV_BOTONES_H
#define DRV_BOTONES_H

#include <stdint.h>
#include "rt_evento_t.h"

/* Tiempos de la FSM antirebote (en ms) */
#define DRV_BOTONES_RETARDO_REBOTE_MS  20
#define DRV_BOTONES_PERIODO_MUESTREO_MS 20

/* Identificadores de eventos internos para alarmas */
typedef enum {
    e_esperando,
    e_rebotes,   
    e_muestreo, 
		e_salida
} Estado_boton;

// Pre:  'callback' debe ser una función válida que será llamada desde la HAL
//       cuando se detecte una interrupción externa.
//       Los módulos HAL (GPIO, EXT_INT), GE y ALARMA deben estar inicializados.
// Post: Los botones quedan configurados como entradas, con interrupciones
//       habilitadas y capaces de despertar el sistema. Se suscriben todos los
//       eventos necesarios y su estado interno queda en e_esperando.
void drv_botones_iniciar(void (*callback)());

// Pre:  Debe haber sido llamado antes drv_botones_iniciar().
//       'evento' será uno de los esperados por la MSF del driver.
//       'auxData' contiene el ID del botón asociado al evento.
// Post: Actualiza el estado interno del botón correspondiente y activa
//       las alarmas o interrupciones necesarias para continuar el proceso.
void drv_botones_actualizar(EVENTO_T evento, uint32_t boton_id);

// Pre:  Debe haberse llamado antes drv_botones_iniciar().
//       'evento' debe ser un evento válido para la MSF de botones.
//       'boton_id' es el identificador del botón cuyo estado se actualiza.
// Post: El estado interno del botón correspondiente se modifica según la MSF,
//       pudiendo activar alarmas, deshabilitar o habilitar interrupciones
//       y generar eventos derivados.
void drv_boton_estado_actualizar(EVENTO_T evento, uint32_t boton_id);

// Retorna la cantidad de botones activos disponibles
unsigned int drv_botones_cantidad(void);

#endif // DRV_BOTONES_H
