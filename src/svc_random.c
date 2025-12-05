/* *****************************************************************************
 * P.H.2025: Implementación del servicio de números aleatorios
 */

#include "svc_random.h"
#include "hal_random.h"

void svc_random_iniciar(uint64_t seed)
{
    hal_random_iniciar(seed);
}

uint32_t svc_random_value(uint32_t min, uint32_t max)
{   
    return hal_random_value(min, max);
}
