/* *****************************************************************************
 * P.H.2025: HAL de consumo, interfaz dependiente del hardware para cambiar el 
 * el estado del procesador
 */
 
#ifndef HAL_CONSUMO
#define HAL_CONSUMO

#include <stdint.h>
#include <stdbool.h>

// Inicializa parámetros necesarios 
void hal_consumo_iniciar(void);

// Deja el procesador en modo espera
void hal_consumo_esperar(void);

// Deja el procesador en modo bajo consumo
void hal_consumo_dormir(void);

#endif // HAL_CONSUMO
