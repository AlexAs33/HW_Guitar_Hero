#include <LPC210x.H>
#include "hal_uart.h"

#define PCLK 15000000

void hal_uart_init(unsigned long baudrate) {
    unsigned int divisor;
    
    PINSEL0 |= 0x00050000;  // P0.8=TXD1, P0.9=RXD1
    
    divisor = PCLK / (16 * baudrate);
    
    U1LCR = 0x83;
    U1DLL = divisor & 0xFF;
    U1DLM = (divisor >> 8) & 0xFF;
    U1LCR = 0x03;
    U1FCR = 0x07;
}

void hal_uart_putchar(char c) {
    while (!(U1LSR & 0x20));
    U1THR = c;
}

char hal_uart_getchar(void) {
    while (!(U1LSR & 0x01));
    return U1RBR;
}

int hal_uart_data_available(void) {
    return (U1LSR & 0x01);
}
