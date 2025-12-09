

#include "hal_SC.h"
#include <LPC210x.H> /* Definiciones espec�ficas del hardware LPC210x */

extern void enable_irq(void);  //habilita las interrupciones globales
extern void disable_irq(void); //heshabilita las interrupciones globales

uint32_t hal_sc_entrar(void) {
    __disable_irq();
	return 1;
}

void hal_sc_salir(void) {
    __enable_irq();
}
