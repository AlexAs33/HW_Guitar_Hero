/* ***************************************************************************************
 * P.H.2025: Interfaz de generación de números [pseudo]aleatorios
 *           Proporciona las funciones necesarias para iniciar y generar números pseudoaleatorios
 *           específicamente para la arquitectura deseada
 *
 * Funciones:
 * - Inicializar el generador con una semilla
 * - Generar números [pseudo]aleatorios en un rango
 * 
 * Nota:
 *  los números seran aleatorios o pseudoaleatorios dependiendo del hardware en el que nos encontremos
 *
 * Autores:
 * - Pablo Plumed
 * - Alex Asensio
 *************************************************************************************** */
#ifndef HAL_RANDOM
#define HAL_RANDOM

#include <stdint.h>

//Pre:  Se recibe un valor seed cualquiera de 64 bits.
//Post: El generador queda inicializado con el seed proporcionado
void hal_random_iniciar(uint64_t seed);

//Pre:  min <= max. El generador debe haber sido inicializado previamente con random_iniciar().
//Post: Devuelve un número pseudoaleatorio en el rango [min, max]
uint32_t hal_random_value(uint32_t min, uint32_t max);

#endif // HAL_RANDOM

