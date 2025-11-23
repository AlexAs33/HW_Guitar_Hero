/* *****************************************************************************
 * P.H.2025: Interfaz para enviar y recibir mensaje via UART del LPC2105
 */

#include <LPC210x.H>
#include "hal_uart.h"

#define PCLK    15000000
#define PIN_TXD 0
#define PIN_RXD 2

void hal_uart_init(unsigned long baudrate) {
    unsigned int divisor;

    // configurar tx y rx 
    PINSEL0 &= ~(3 << PIN_TXD);
    PINSEL0 |=  (1 << PIN_TXD);
    PINSEL0 &= ~(3 << PIN_RXD);
    PINSEL0 |=  (1 << PIN_RXD);

    divisor = PCLK / (16 * baudrate);

    // 8 bits de datos, 1 de stop y divisor activado
    U0LCR = 0x83;
    // configuración del baudrate para la parte baja y alta
    U0DLL = divisor & 0xFF;
    U0DLM = (divisor >> 8) & 0xFF;
    // desactiva dlab
    U0LCR = 0x03;
    // activa y limpia las fifos
    U0FCR = 0x07;
}

void hal_uart_putchar(char c) {
    while (!(U0LSR & 0x20));
    U0THR = c;
}

char hal_uart_getchar(void) {
    while (!(U0LSR & 0x01));
    return U0RBR;
}

int hal_uart_data_available(void) {
    return (U0LSR & 0x01);
}
