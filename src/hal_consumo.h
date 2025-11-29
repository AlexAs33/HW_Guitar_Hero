/* *****************************************************************************
 * P.H.2025: Interfaz HAL de consumo
 *
 * Proporciona una interfaz de alto nivel para gestionar los modos de energía
 * del sistema. Permite iniciar el módulo de consumo, poner el procesador en
 * modo de espera y activar el modo de bajo consumo,
 * garantizando que el sistema pueda despertar correctamente mediante eventos
 * externos habilitados.
 *
 * Funciones:
 * - Inicializar el sistema en modo de funcionamiento normal
 * - Entrar en modo de espera
 * - Entrar en modo de bajo consumo
 *
 * Autores:
 * - Pablo Plumed
 * - Alex Asensio
 ***************************************************************************** */

 
#ifndef HAL_CONSUMO
#define HAL_CONSUMO

#include <stdint.h>
#include <stdbool.h>

// Pre: --
// Post: El sistema queda configurado en modo de funcionamiento normal y
//       preparado para poder entrar posteriormente en modos de bajo consumo. 
void hal_consumo_iniciar(void);

// Pre:  El sistema debe estar inicializado mediante hal_consumo_iniciar().
// Post: El procesador entra en modo de espera. Se reanudará
//       automáticamente cuando ocurra un evento externo habilitado.
void hal_consumo_esperar(void);

// Pre:  El sistema debe estar inicializado mediante hal_consumo_iniciar().
// Post: El procesador entra en modo de bajo consumo. Se 
//       reanudará cuando ocurra un evento externo habilitado y quedará listo
//       para continuar la ejecución normal.
void hal_consumo_dormir(void);

#endif // HAL_CONSUMO
