/* *****************************************************************************
 * P.H.2025: Implementación de números aleatorios para el LPC
 * Este fichero utiliza la API ofrecida opr la standard library de C
 */

/* *****************************************************************************
 * P.H.2025: Implementación de números aleatorios para el NRF
 * Este fichero utiliza la API ofrecida por el hardware dedicado del procesador
 */

#include "hal_random.h"
#include "nrf.h"

void hal_random_iniciar(uint64_t seed)
{
		// no es necesaria la seed en nrf
    (void)seed;
    NRF_RNG->CONFIG = RNG_CONFIG_DERCEN_Enabled << RNG_CONFIG_DERCEN_Pos;
    NRF_RNG->EVENTS_VALRDY = 0;
    NRF_RNG->TASKS_START = 1;
}

static uint8_t random8(void)
{
    // espera a que haya un byte listo
    while (NRF_RNG->EVENTS_VALRDY == 0);
		// limpia evento
    NRF_RNG->EVENTS_VALRDY = 0;    
    return NRF_RNG->VALUE;          
}

uint32_t hal_random_value(uint32_t min, uint32_t max)
{
    // intercambiamos para q el rango no de problemas
    if (min > max) {
        min ^= max;
        max ^= min;
        min ^= max;
    }

    // construimos número de 32 bits
    uint32_t r =
        ((uint32_t)random8() << 24) |
        ((uint32_t)random8() << 16) |
        ((uint32_t)random8() << 8)  |
        ((uint32_t)random8());

    return min + (r % (max - min + 1));
}
