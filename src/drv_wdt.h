/* ***************************************************************************************
 * P.H.2025: Interfaz del Driver para el Watchdog Timer
 * Este archivo define la interfaz para la gestión de un WDT. 
 * Permite inicializar y alimentar periódicamente el WDT para evitar reinicios del sistema.
 *
 * Funciones:
 *  - Inicializar el WDT con un tiempo de timeout configurable.
 *  - Resetear el WDT.
 *
 * Autores:
 * - Pablo Plumed
 * - Alex Asensio
 *************************************************************************************** */

#ifndef DRV_WDT_H
#define DRV_WDT_H

#include <stdint.h>

//Pre: tsec -> el tiempo en segundos en el que saltará el WDT
//Post: se inicializa el WDT para saltar en tsec segundos
void drv_wdt_iniciar(uint32_t tsec);

//Pre: --
//Post: se resetea el WDT
void drv_wdt_feed(void);

#endif	//DRV_WDT_H
