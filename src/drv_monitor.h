/* *****************************************************************************
 * P.H.2025: Interfaz del Driver de Monitores
 * 
 * Este módulo proporciona una interfaz de alto nivel para controlar los
 * monitores del sistema. Los monitores son pines GPIO utilizados para marcar
 * eventos, estados o secciones críticas del programa, facilitando su
 * observación mediante un analizador lógico.
 *
 * Funciones:
 * - Inicializar todos los monitores como salidas desmarcadas
 * - Marcar un monitor (ponerlo a nivel lógico activo)
 * - Desmarcar un monitor (ponerlo a nivel lógico inactivo)
 *
 * Autores:
 * - Pablo Plumed
 * - Alex Asensio
 ***************************************************************************** */

#ifndef DRV_MONITOR_H
#define DRV_MONITOR_H

#include <stdint.h>
#include "board.h"
#include "hal_gpio.h"

typedef uint32_t MONITOR_id_t;

// Pre:  El sistema debe haber inicializado previamente el HAL de GPIO.
// Post: Todos los monitores quedan configurados como salida y desactivados.
//       Devuelve el número total de monitores disponibles.
uint32_t drv_monitor_iniciar(void); 

// Pre:  'id' debe corresponder a un monitor válido (1..MONITORS_NUMBER).
//       El módulo debe haber sido inicializado mediante drv_monitor_iniciar().
// Post: El monitor indicado queda marcado. Devuelve 1 si la operación es válida
//       o 0 si el id es incorrecto o no existen monitores configurados.
int drv_monitor_marcar(MONITOR_id_t id);

// Pre:  'id' debe corresponder a un monitor válido (1..MONITORS_NUMBER).
//       El módulo debe haber sido inicializado mediante drv_monitor_iniciar().
// Post: El monitor indicado queda desmarcado. Devuelve 1 si la operación es válida
//       o 0 si el id es incorrecto o no existen monitores configurados.
int drv_monitor_desmarcar(MONITOR_id_t id);

#endif  //DRV_MONITOR_H
