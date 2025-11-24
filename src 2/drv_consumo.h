/* *****************************************************************************
 * P.H.2025: Driver/Manejador de tiempo (independiente de HW)
 * Mantiene concepto: tiempo absoluto en us/ms + espera bloqueante y hasta �deadline�.
 */
#ifndef DRV_CONSUMO_H
#define DRV_CONSUMO_H

#include <stdint.h>
#include <stdbool.h>
#include "drv_monitor.h"

// Inicializa el driver de consumo, indicando el monitor a usar
void drv_consumo_iniciar(MONITOR_id_t id);

// Poner el micro en modo de espera 
void drv_consumo_esperar(void);

// Poner el micro en modo de bajo consumo
void drv_consumo_dormir(void);


#endif // DRV_CONSUMO_H
