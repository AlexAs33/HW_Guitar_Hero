/* ***************************************************************************************
 * P.H.2025: Interfaz HAL para el Watchdog Timer
 * Este archivo define la interfaz HAL para la gestión del WDT en sistemas embebidos. 
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

#ifndef HAL_WDT_H
#define HAL_WDT_H

#include <stdint.h>


//Pre: tsec -> el tiempo en segundos en el que saltará el WDT
//Post: se inicializa el WDT para saltar en tsec segundos
void hal_wdt_iniciar(uint32_t tsec);

//Pre: --
//Post: se resetea el WDT
void hal_wdt_feed(void);

#endif	//HAL_WDT_H
