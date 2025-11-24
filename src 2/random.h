/* *****************************************************************************
 * P.H.2025: Generador de números pseudoaleatorios
 */

#ifndef RANDOM
#define RANDOM

#include <stdint.h>

// Inicializa la creación de números pseudoaleatorios a partir de seed
void random_iniciar(uint64_t seed);

// Devuelve un valor pseudoaleatorio entre min y max
uint32_t random_value(uint32_t min, uint32_t max);

#endif // RANDOM
