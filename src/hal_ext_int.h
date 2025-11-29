/* ***************************************************************************************
 * P.H.2025: Interfaz HAL para las interrupciones externas
 * Proporciona la interfaz necesaria para iniciar, leer, habilitar, deshabilitar
 * y permitir que determinadas interrupciones externas puedan despertar al sistema.
 *
 * Funciones:
 * - Iniciar el subsistema de interrupciones externas
 * - Leer el estado de una interrupción externa
 * - Habilitar una interrupción externa
 * - Deshabilitar una interrupción externa
 * - Habilitar una interrupción como fuente de despertar
 * - Deshabilitar una interrupción como fuente de despertar
 * - Limpiar flags de interrupción pendientes (individual o global)
 * - Cambiar un pin entre modo GPIO y modo interrupción externa
 *
 * Autores:
 * - Pablo Plumed
 * - Alex Asensio
 *************************************************************************************** */


#ifndef HAL_EXT_INT_H
#define HAL_EXT_INT_H

#include <stdint.h>
#include "board.h"

// Pre:  El módulo no debe estar inicializado previamente. El callback debe ser
//       una función válida o NULL si no se desea callback.
// Post: El sistema de interrupciones externas queda inicializado y el callback
//       queda registrado para ser llamado al producirse una interrupción.
void hal_ext_int_iniciar(void (*callback)());

// Pre:  El pin debe corresponder a uno válido (EINT0, EINT1 o EINT2).
// Post: Devuelve 1 si la interrupción correspondiente está activa, 0 si no lo está,
//       y -1 si el pin no es válido.
int32_t hal_ext_int_leer(uint32_t pin); 

// Pre:  El pin debe corresponder a un pin válido de interrupción externa.
//       El módulo debe haber sido inicializado previamente.
// Post: La interrupción asociada al pin queda habilitada.
void hal_ext_int_habilitar_int(uint32_t pin);

// Pre:  El pin debe corresponder a un pin válido de interrupción externa.
//       El módulo debe haber sido inicializado previamente.
// Post: La interrupción asociada al pin queda deshabilitada.
void hal_ext_int_deshabilitar_int(uint32_t pin);

// Pre:  El pin debe ser un pin válido de interrupción externa.
// Post: El pin pasa a poder despertar al sistema desde bajo consumo.
void hal_ext_int_habilitar_despertar(uint32_t pin);

// Pre:  El pin debe ser un pin válido de interrupción externa.
// Post: El pin deja de poder despertar al sistema.
void hal_ext_int_deshabilitar_despertar(uint32_t pin);

// Pre:  El pin debe ser un pin válido de interrupción externa.
// Post: El flag asociado al pin queda limpiado.
void hal_ext_int_clear_flag(uint32_t pin);

// Pre:  --
// Post: Todas las interrupciones externas pendientes quedan limpiadas.
void hal_ext_int_clear(void);

// Pre:  El pin debe ser uno de los pines configurables como EINT.
// Post: El pin queda configurado como GPIO sin función alternativa.
void pin_to_gpio(uint32_t pin);

// Pre:  El pin debe ser uno de los pines válidos para interrupción externa.
// Post: El pin queda configurado para generar interrupciones externas.
void pin_to_eint(uint32_t pin);

#endif // HAL_EXT_INT_H
