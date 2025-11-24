

#include "hal_wdt.h"
#include <NRF.H>


void hal_wdt_iniciar(uint32_t tsec) {
  // Calcula el valor del registro CRV para el tiempo de timeout:
  // CRV = (timeout en segundos) * (frecuencia del reloj LFCLK, típicamente 32.768 Hz) - 1.
  uint32_t crv_value = (tsec * 32768) - 1;

  // Configura el WDT para funcionar tanto en estado HALT como SLEEP.
  NRF_WDT->CONFIG = (WDT_CONFIG_SLEEP_Msk | WDT_CONFIG_HALT_Msk);

  // Establece el valor del timeout en el registro CRV.
  NRF_WDT->CRV = crv_value;

  // Habilita el canal RR0 (reload request 0) para alimentar el WDT.
  NRF_WDT->RREN |= WDT_RREN_RR0_Msk;

  // Inicia el WDT escribiendo 1 en el registro TASKS_START.
  NRF_WDT->TASKS_START = 1;
}


void hal_wdt_feed() {
  NRF_WDT->RR[0] = 0x6E524635; // TODO mirar si cambiar Valor mágico para alimentar el WDT.
}
