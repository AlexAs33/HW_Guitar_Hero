/* *****************************************************************************
 * P.H.2025: Driver/Manejador de monitores (independiente de HW)
 * Ofrece la API encargada de monitorizar la ejecuci�n del programa
 */
#ifndef DRV_MONITOR_H
#define DRV_MONITOR_H

#include <stdint.h>
#include "board.h"
#include "hal_gpio.h"

typedef uint32_t MONITOR_id_t;

/* Inicializa todos los monitores (como salidas, desmarcados) */
uint32_t drv_monitor_iniciar(void); 

/* Marca (pone a 1) el monitor indicado */
int drv_monitor_marcar(MONITOR_id_t id);

/* Desmarca (pone a 0) el monitor indicado */
int drv_monitor_desmarcar(MONITOR_id_t id);

#endif /* DRV_MONITOR_H */
