/* ***************************************************************************************
 * P.H.2025: Interfaz HAL para la Sclusion Zone
 * Este archivo define la interfaz HAL para la gesti�n de una zona de esclusi�n.
 * Abstrae las operaciones que permite activar y desactivar interrupciones cr�ticas
 * de forma segura en el contexto de una ejecuci�n concurrente.
 *
 * Funciones:
 *  - Desabilitar las interrupciones para entrar en la zona de exclusi�n.
 *  - Restaurar el sistema al salir de la zona de exclusi�n.
 *
 * Autores:
 * - Pablo Plumed
 * - Alex Asensio
 *************************************************************************************** */

#ifndef HAL_SC_H
#define HAL_SC_H

#include <stdint.h>

//Pre: --
//Post: Asegura que no se van a procesar interrupciones.
//			Devuelve 1 si las interrupciones es�n habilitadas. 0 en caso contrario
uint32_t hal_sc_entrar(void);

//Pre: --
//Post: Devuelve el el sistema al estado anterior. Se procesar�n interrupciones
void hal_sc_salir(void);

#endif	//HAL_SC_H
