/* ***************************************************************************************
 * P.H.2025: Implementacion del Driver de consumo
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
 
#include "drv_consumo.h"
#include "hal_consumo.h"
#include "hal_ext_int.h"

#ifdef DEBUG
    #include "svc_estadisticas.h"
#endif

MONITOR_id_t  monitor_id = 0;

void drv_consumo_iniciar(MONITOR_id_t id) {
	hal_consumo_iniciar();
	drv_monitor_iniciar();
	monitor_id = id;
}

void drv_consumo_esperar(){	
#ifdef DEBUG 
						svc_estadisticas_set_tmp(e_TIEMPO_FIN_DESPIERTO);
						svc_estadisticas_set_tmp(e_TIEMPO_INICIO_ESPERA);
#endif   
	
	drv_monitor_desmarcar(monitor_id);
	hal_consumo_esperar();
	drv_monitor_marcar(monitor_id);
	
#ifdef DEBUG 
						svc_estadisticas_set_tmp(e_TIEMPO_FIN_ESPERA);
						svc_estadisticas_set_tmp(e_TIEMPO_INICIO_DESPIERTO);
#endif   
	
}

void drv_consumo_dormir(){
#ifdef DEBUG 
						svc_estadisticas_set_tmp(e_TIEMPO_FIN_DESPIERTO);
						svc_estadisticas_set_tmp(e_TIEMPO_INICIO_ESPERA);
#endif  
	
	hal_ext_int_clear();
	hal_ext_int_clear();
	hal_consumo_dormir();
	drv_monitor_marcar(monitor_id);
	
#ifdef DEBUG 
						svc_estadisticas_set_tmp(e_TIEMPO_FIN_ESPERA);
						svc_estadisticas_set_tmp(e_TIEMPO_INICIO_DESPIERTO);
#endif   
	
}
