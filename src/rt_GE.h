/* ***************************************************************************************
 * P.H.2025: Interfaz del Runtime de Gestión de Eventos
 * Proporciona la interfaz necesaria para iniciar, lanzar y actualizar eventos asíncronos
 *
 * Funciones:
 * - Inicializar el gestor de eventos
 * - Poner en marcha el gestor de eventos
 * - Actualizar eventos
 *
 * Autores:
 * - Pablo Plumed
 * - Alex Asensio
 *************************************************************************************** */

#ifndef RT_GE_H
#define RT_GE_H

#include <stdint.h>
#include <stdbool.h>
#include "rt_evento_t.h"
#include "drv_monitor.h"

//Variables del gestor de eventos
#define rt_GE_MAX_SUSCRITOS 4								//Máximo número de eventos
#define rt_GE_TIMEOUT_INACTIVIDAD_MS 25000  //Tiempo de inactividad máximo

//Pre: --
//Post: inicializa el gestor de eventos.
//			el monitor se activa cuando hay mas eventos de los disponibles
void rt_GE_iniciar(MONITOR_id_t M_overflow);

//Pre: --
//Post: pone en marcha el gestor de eventos
void rt_GE_lanzador(void);

//Actualiza el evento.
//Gestiona los tiempos de inactividad
void rt_GE_actualizar(EVENTO_T evento, uint32_t auxiliar);

#endif // RT_GE_H
