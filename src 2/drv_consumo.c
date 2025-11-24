/* *****************************************************************************
 * P.H.2025: Driver/Manejador del consumo
 * suministra los servicios independientemente del hardware
 *
 * usa los servicos de hal_consumo.h: 
 */
 
#include "drv_consumo.h"
#include "hal_consumo.h"
#include "hal_ext_int.h"

MONITOR_id_t  monitor_id = 0;

void drv_consumo_iniciar(MONITOR_id_t id) {
	hal_consumo_iniciar();
	drv_monitor_iniciar();
	monitor_id = id;
}

void drv_consumo_esperar(){
	drv_monitor_desmarcar(monitor_id);
	hal_consumo_esperar();
	drv_monitor_marcar(monitor_id);
}

void drv_consumo_dormir(){
	hal_ext_int_clear();
	//drv_monitor_desmarcar(monitor_id);
	hal_ext_int_clear();
	hal_consumo_dormir();
	drv_monitor_marcar(monitor_id);
}
