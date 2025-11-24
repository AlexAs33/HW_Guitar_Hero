/* *****************************************************************************
 * P.H.2025: Interfaz para enviar y recibir mensaje via UART del nrf52840
 */

#include <nrf.h>
#include "hal_uart.h"

#define PIN_TXD 6
#define PIN_RXD 8

static uint8_t tx_buf;

uint32_t uarte_baudrate(unsigned long baudrate) {
    uint32_t uarte_baud;
    switch (baudrate) {
        case 1200:   uarte_baud = UARTE_BAUDRATE_BAUDRATE_Baud1200; break;
        case 2400:   uarte_baud = UARTE_BAUDRATE_BAUDRATE_Baud2400; break;
        case 4800:   uarte_baud = UARTE_BAUDRATE_BAUDRATE_Baud4800; break;
        case 9600:   uarte_baud = UARTE_BAUDRATE_BAUDRATE_Baud9600; break;
        case 14400:  uarte_baud = UARTE_BAUDRATE_BAUDRATE_Baud14400; break;
        case 19200:  uarte_baud = UARTE_BAUDRATE_BAUDRATE_Baud19200; break;
        case 38400:  uarte_baud = UARTE_BAUDRATE_BAUDRATE_Baud38400; break;
        case 57600:  uarte_baud = UARTE_BAUDRATE_BAUDRATE_Baud57600; break;
        case 115200: uarte_baud = UARTE_BAUDRATE_BAUDRATE_Baud115200; break;
        case 230400: uarte_baud = UARTE_BAUDRATE_BAUDRATE_Baud230400; break;
        case 460800: uarte_baud = UARTE_BAUDRATE_BAUDRATE_Baud460800; break;
        case 921600: uarte_baud = UARTE_BAUDRATE_BAUDRATE_Baud921600; break;
        default:     uarte_baud = UARTE_BAUDRATE_BAUDRATE_Baud115200; break; 
    }
    return uarte_baud;
}

void hal_uart_init(unsigned long baudrate) {
    // configurar UART sin flow control, con un bit de paridad y baudrate
   // NRF_UARTE0->CONFIG = (UART_CONFIG_HWFC_Disabled   << UART_CONFIG_HWFC_Pos) |
     //                   (UART_CONFIG_PARITY_Included << UART_CONFIG_PARITY_Pos);
	
	NRF_UARTE0->CONFIG = 0;

    NRF_UARTE0->BAUDRATE = uarte_baudrate(baudrate) << UARTE_BAUDRATE_BAUDRATE_Pos;

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

    tx_buf = (uint8_t)c;                 // Copiamos el byte en un buffer seguro

    NRF_UARTE0->TXD.PTR = (uint32_t)&tx_buf;  // El DMA lee SIEMPRE de aquí
    NRF_UARTE0->TXD.MAXCNT = 1;
    NRF_UARTE0->EVENTS_ENDTX = 0;
    NRF_UARTE0->TASKS_STARTTX = 1;

    while (NRF_UARTE0->EVENTS_ENDTX == 0);
    NRF_UARTE0->EVENTS_ENDTX = 0;
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
