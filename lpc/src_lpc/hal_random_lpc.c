/* *****************************************************************************
 * P.H.2025: Implementación de números aleatorios para el LPC
 * Este fichero utiliza la API ofrecida opr la standard library de C
 */

#include "hal_random.h"
#include <stdlib.h>

void hal_random_iniciar(uint64_t seed)
{
    // Inicializa la semilla
    srand(seed);
}

uint32_t hal_random_value(uint32_t min, uint32_t max)
{
    // Intercambia valores si están las revés
    if (min > max) {
        min ^= max;
        max = min ^ max;
        min ^= max;
    }
    return min + (rand() % (max - min + 1));
}
