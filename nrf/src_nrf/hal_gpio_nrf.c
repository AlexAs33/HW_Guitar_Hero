/* *****************************************************************************
 * P.H.2025: TODO
 */
 

#include <nrf.h>                       /* nRF 0x definitions */

#include "hal_gpio.h"

/**

 */
 
void hal_gpio_iniciar(void){
  // Reiniciamos los pines todos como salida (igual al reset):
  NRF_P0->DIR = 0x0000000F; // GPIO Port Direction control register.
				       // Controla la direcci�n de cada puerto pin
}

/* *****************************************************************************
 * Acceso a los GPIOs 
 */

/**
 * El gpio se configuran como entrada o salida seg�n la direcci�n.
 */
void hal_gpio_sentido(HAL_GPIO_PIN_T gpio, hal_gpio_pin_dir_t direccion){
	uint32_t masc = (1UL << gpio);
	if (direccion == HAL_GPIO_PIN_DIR_INPUT)
   	    NRF_P0->DIRCLR = masc;
	else
        NRF_P0->DIRSET = masc;	
}
 
/**
 * La funci�n devuelve un entero (bool) con el valor de los bits indicados.
 */
uint32_t hal_gpio_leer(HAL_GPIO_PIN_T gpio){
	uint32_t masc = (1UL << gpio);
	return (NRF_P0->OUT & masc) ? 1u : 0u;
}

// Leer pin de entrada
uint32_t hal_gpio_leer_in(HAL_GPIO_PIN_T gpio){
    uint32_t masc = (1UL << gpio);
    return (NRF_P0->IN & masc) ? 1u : 0u;   
}



/**
 * Escribe en el gpio el valor
 */
void hal_gpio_escribir(HAL_GPIO_PIN_T gpio, uint32_t valor){
	uint32_t masc = (1UL << gpio);
	
	if ((valor & 0x01) == 1) NRF_P0->OUTSET = masc;
	else NRF_P0->OUTCLR = masc;
}
