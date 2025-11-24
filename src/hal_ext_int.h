#ifndef HAL_EXT_INT_H
#define HAL_EXT_INT_H

#include <stdint.h>
#include "board.h"

/**
 * Inicializa el subsistema de interrupciones externas
 */
void hal_ext_int_iniciar(void (*callback)());

/*
 * Lee el estado de la interrupción externa en pin
 */
int32_t hal_ext_int_leer(uint32_t pin); 

/**
 * Habilita la interrupción externa del pin
 */
void hal_ext_int_habilitar_int(uint32_t pin);

/**
 * Deshabilita la interrupción externa del pin 
 */
void hal_ext_int_deshabilitar_int(uint32_t pin);

/**
 * Habilita la interrupción externa para despertar al sistema 
 */
void hal_ext_int_habilitar_despertar(uint32_t pin);

/**
 * Deshabilita la interrupción externa para despertar al sistema
 */
void hal_ext_int_deshabilitar_despertar(uint32_t pin);
/**
 * Limpia el flag de interrupción pendiente
 */
void hal_ext_int_clear_flag(uint32_t pin);

/**
 * Limpia todos los flags de interrupción pendientes
 */
void hal_ext_int_clear(void);

void pin_to_gpio(uint32_t pin);

void pin_to_eint(uint32_t pin);

#endif // HAL_EXT_INT_H
