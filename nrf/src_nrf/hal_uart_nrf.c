/* *****************************************************************************
 * P.H.2025: Interfaz para enviar y recibir mensaje via UART del nrf52840
 */

#include <nrf.h>
#include "hal_uart.h"

#define PIN_TXD 6
#define PIN_RXD 8

void hal_uart_init(unsigned long baudrate) {
    // configurar UART sin flow control, con un bit de paridad y baudrate de 115200
    NRF_UARTE0->CONFIG = (UART_CONFIG_HWFC_Disabled   << UART_CONFIG_HWFC_Pos) |
                        (UART_CONFIG_PARITY_Included << UART_CONFIG_PARITY_Pos);

    NRF_UARTE0->BAUDRATE = UARTE_BAUDRATE_BAUDRATE_Baud115200 << UARTE_BAUDRATE_BAUDRATE_Pos;

    // seleccionar pins de transfer y receive
    NRF_UARTE0->PSEL.TXD = PIN_TXD;
    NRF_UARTE0->PSEL.RXD = PIN_RXD;

    // enable de la UART
    NRF_UARTE0->ENABLE = UARTE_ENABLE_ENABLE_Enabled << UARTE_ENABLE_ENABLE_Pos;

    // inicializar rx y tx para que putchar y getchar puedan funcionar
    NRF_UARTE0->EVENTS_ENDTX = 0;
    NRF_UARTE0->EVENTS_ENDRX = 0;
}

void hal_uart_putchar(char c) {
    NRF_UARTE0->TXD.PTR = (uint32_t)&c;   // apunta al dato
    NRF_UARTE0->TXD.MAXCNT = 1;           // 1 byte
    NRF_UARTE0->EVENTS_ENDTX = 0;         // limpia evento
    NRF_UARTE0->TASKS_STARTTX = 1;        // comienza a transmitir

    // esperar a que acabe, pero con timeout para evitar bloqueo infinito
    volatile int timeout = 1000000; 
    while (NRF_UARTE0->EVENTS_ENDTX == 0 && timeout > 0) timeout--;  
    NRF_UARTE0->EVENTS_ENDTX = 0;         // termina de transmitir
}

char hal_uart_getchar() {
    // idem al anterior
    uint8_t c;
    NRF_UARTE0->RXD.PTR = (uint32_t)&c;
    NRF_UARTE0->RXD.MAXCNT = 1;
    NRF_UARTE0->EVENTS_ENDRX = 0;
    NRF_UARTE0->TASKS_STARTRX = 1;

    // espera a que llegue un byte
    while (NRF_UARTE0->EVENTS_ENDRX == 0);
    NRF_UARTE0->EVENTS_ENDRX = 0;

    return c;
}

// comprueba si hay datos disponibles
int hal_uart_data_available() {
    return (NRF_UARTE0->EVENTS_ENDRX != 0);
}
