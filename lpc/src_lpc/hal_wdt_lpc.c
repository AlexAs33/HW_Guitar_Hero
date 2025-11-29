

#include "hal_wdt.h"
#include "drv_sc.h"
#include <LPC210x.H> /* Definiciones específicas del hardware LPC210x */

void hal_wdt_iniciar(uint32_t tsec) {
  // Configura el tiempo de timeout en el registro WDTC.
  // WDTC requiere el tiempo en ciclos de reloj del WDT (PCLK / 4).
  WDTC = tsec * (15000000 / 4);

  // Configuración del WDT:
  // b0 (WDEN) = 1: Habilita el WDT.
  // b1 (WDRESET) = 1: Habilita el reinicio del sistema al expirar el WDT.
  WDMOD = 0x03;

  // Alimenta el WDT inicialmente para evitar un reinicio inmediato.
  hal_wdt_feed();
}

void hal_wdt_feed(void) {
	drv_sc_disable();
  WDFEED = 0xAA; // Primera escritura requerida para alimentar el WDT.
  WDFEED = 0x55; // Segunda escritura requerida para alimentar el WDT.
	drv_sc_enable();
}
