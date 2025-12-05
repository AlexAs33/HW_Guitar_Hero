/* ***************************************************************************************
 * P.H.2025: Interfaz de generación de números [pseudo]aleatorios
 *           Proporciona las funciones necesarias para iniciar y gennerar estos números
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
#ifndef SVC_RANDOM
#define SVC_RANDOM

#include <stdint.h>

//Pre:  Se recibe un valor seed cualquiera de 64 bits.
//Post: El generador queda inicializado con el seed proporcionado
void svc_random_iniciar(uint64_t seed);

//Pre:  min <= max. El generador debe haber sido inicializado previamente con random_iniciar().
//Post: Devuelve un número pseudoaleatorio en el rango [min, max]
uint32_t svc_random_value(uint32_t min, uint32_t max);

#endif // SVC_RANDOM
