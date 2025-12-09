/* ***************************************************************************************
 * P.H.2025: Interfaz del Driver para la Sclusion Zone
 * Este archivo define la interfaz para la gestión de una zona de esclusión.
 * Abstrae las operaciones que permite activar y desactivar interrupciones críticas
 * de forma segura en el contexto de una ejecución concurrente.
 *
 * Funciones:
 *  - Habilitar las interrupciones al salir de la zona de exclusión.
 *  - Desabilitar las interrupciones para entrar en la zona de exclusión.
 *
 * Autores:
 * - Pablo Plumed
 * - Alex Asensio
 *************************************************************************************** */

#ifndef DRV_SC_H
#define DRV_SC_H

#include <stdint.h>

//Pre: --
//Post: decrementa el anidamiento. Solo las habilita si el nivel es 0.
void drv_sc_entrar(void);

//Pre: --
//Post: cuenta el nivel de anidamiento de zonas de exclusión.
//			Solo deshabilita las interrupciones si el nivel es 0.
void drv_sc_salir(void);

#endif	//DRV_SC_H
