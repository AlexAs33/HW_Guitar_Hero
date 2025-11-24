/* ***************************************************************************************
 * P.H.2025: Interfaz del Servicio de Gestión de Eventos
 * Proporciona la interfaz necesaria para suscribir y cancelar eventos asíncronos
 *
 * Funciones:
 * - Suscribir eventos con una prioridad y tarea asociadas
 * - Cancelar tareas asociadas a eventos
 *
 * Autores:
 * - Pablo Plumed
 * - Alex Asensio
 *************************************************************************************** */
 
#ifndef SVC_GE_H
#define SVC_GE_H

#include <stdint.h>
#include <stdbool.h>
#include "rt_evento_t.h"

//Pre: evento existente
//Post: asocia una tarea a un evento con una prioridad
bool svc_GE_suscribir(EVENTO_T ID_evento, uint8_t prioridad, void (*f_callback)(EVENTO_T, uint32_t));

//Pre: evento existente
//Post: cancela la tarea asociada a un evento
void svc_GE_cancelar(EVENTO_T ID_evento, void (*f_callback)(EVENTO_T, uint32_t));

#endif //SVC_GE_H
