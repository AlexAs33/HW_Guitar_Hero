/* ***************************************************************************************
 * P.H.2025: Implementación del Driver para el Watchdog Timer
 *
 * Funciones:
 *  - Inicializar el WDT con un tiempo de timeout configurable.
 *  - Resetear el WDT.
 *
 * Autores:
 * - Pablo Plumed
 * - Alex Asensio
 *************************************************************************************** */

#include "drv_wdt.h"

#include "hal_wdt.h"
#include "drv_sc.h"

//Inicializar el WDT con un timeout de tsec segundos
void drv_wdt_iniciar(uint32_t tsec){
    drv_sc_entrar();
    hal_wdt_iniciar(tsec);
    drv_sc_salir();
}

//Reiniciar el timeout del WDT
void drv_wdt_feed(void){
    drv_sc_entrar();
    hal_wdt_feed();
    drv_sc_salir();
}
