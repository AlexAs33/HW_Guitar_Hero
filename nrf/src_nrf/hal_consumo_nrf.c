/* *****************************************************************************
 * P.H.2025: HAL de consumo
 */


#include <nrf.h>
#include "hal_consumo.h"


void hal_consumo_iniciar(void){
	return;
}

void hal_consumo_esperar(){
	__WFI();
}

void hal_consumo_dormir(){
	NRF_POWER->RESETREAS = 0xFFFFFFFF;
	NRF_POWER->SYSTEMOFF = 1;
	__SEV();
	__WFE();
	__WFE();
}
