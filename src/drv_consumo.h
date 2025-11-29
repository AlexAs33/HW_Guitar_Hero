/* ***************************************************************************************
 * P.H.2025: Driver de consumo
 *
 * Proporciona una interfazpara gestionar los modos de bajo
 * consumo del microcontrolador, integrándose con los monitores y las
 * interrupciones externas.
 *
 * Funcionalidad:
 *  - Inicialización del subsistema de consumo.
 *  - Entrada en modo de espera.
 *  - Entrada en modo de bajo consumo profundo.
 * 
 * Autores:
 * - Pablo Plumed
 * - Alex Asensio
 *************************************************************************************** */

#ifndef DRV_CONSUMO_H
#define DRV_CONSUMO_H

#include <stdint.h>
#include <stdbool.h>
#include "drv_monitor.h"

// Pre:  'id' debe ser un identificador de monitor válido (1..MONITORS_NUMBER).
//       Los módulos HAL relacionados (GPIO, consumo, monitores) deben estar
//       disponibles en el sistema.
// Post: El sistema queda inicializado para gestión de energía. Los monitores
//       quedan iniciados y el monitor indicado se guarda internamente para
//       marcar/desmarcar al entrar y salir de modos de bajo consumo.
void drv_consumo_iniciar(MONITOR_id_t id);

// Pre:  El driver debe haber sido inicializado mediante drv_consumo_iniciar().
//       El monitor asociado debe ser válido.
// Post: El monitor configurado se desmarca antes de entrar en espera.
//       Tras salir, el monitor vuelve a marcarse. 
void drv_consumo_esperar(void);

// Pre:  El driver debe haber sido inicializado mediante drv_consumo_iniciar().
//       Debe existir un monitor válido asignado.
// Post: Se limpian las interrupciones externas pendientes.
//       El procesador entra en modo de bajo consumo.
//       Tras despertar, el monitor asignado queda marcado.
void drv_consumo_dormir(void);


#endif  //DRV_CONSUMO_H
