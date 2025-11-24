

#include "hal_sc.h"
#include <NRF.H> /* Definiciones específicas del hardware nRF */

uint32_t hal_sc_disable(void) {
    uint32_t primask;  //valor actual del PRIMASK.
    uint32_t state;

    primask = __get_PRIMASK();  //0 habilitadas, 1 dehabilitadas

	state = (primask == 0); //guarda estado previo interupciones para devolverlo al final
	
    __disable_irq();    //deshabilita las interrupciones globales escribiendo 1 en PRIMASK.
    
	return state;
}

void hal_sc_restaurar(void) {	
    __enable_irq();   //habilita las interrupciones globales escribiendo 0 en PRIMASK.
}
