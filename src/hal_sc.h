/* ***************************************************************************************
 * P.H.2025: Interfaz HAL para la Sclusion Zone
 * Este archivo define la interfaz HAL para la gestión de una zona de esclusión.
 * Abstrae las operaciones que permite activar y desactivar interrupciones críticas
 * de forma segura en el contexto de una ejecución concurrente.
 *
 * Funciones:
 *  - Desabilitar las interrupciones para entrar en la zona de exclusión.
 *  - Restaurar el sistema al salir de la zona de exclusión.
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
//			Devuelve 1 si las interrupciones esán habilitadas. 0 en caso contrario
uint32_t hal_sc_disable(void);

//Pre: --
//Post: Devuelve el el sistema al estado anterior. Se procesarán interrupciones
void hal_sc_restaurar(void);

#endif	//HAL_SC_H
