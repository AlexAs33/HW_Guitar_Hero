/* *****************************************************************************
 * P.H.2025: Implementacion del Driver de la UART
 *
 * Funciones:
 * - Inicializar la UART con un baudrate específico
 * - Enviar un carácter
 * - Recibir un carácter
 * - Enviar una cadena de caracteres
 * - Enviar un número entero
 * - Consultar si hay datos disponibles en la UART
 *
 * Autores:
 * - Pablo Plumed
 * - Alex Asensio
 ***************************************************************************** */

#include "drv_uart.h"
#include "hal_uart.h"

void drv_uart_init(unsigned long baudrate) {
    hal_uart_init(baudrate);
}

void drv_uart_putchar(char c) {
    hal_uart_putchar(c);
}

void drv_uart_puts(char *s) {
    while (*s) {
        hal_uart_putchar(*s++);
    }
}

void drv_uart_putint(int valor) {
    char buffer[12];
    int i = 0;
    int negativo = 0;
    
    if (valor < 0) {
        negativo = 1;
        valor = -valor;
    }
    
    if (valor == 0) {
        drv_uart_putchar('0');
        return;
    }
    
    while (valor > 0) {
        buffer[i++] = (valor % 10) + '0';
        valor /= 10;
    }
    
    if (negativo) {
        drv_uart_putchar('-');
    }
    
    while (i > 0) {
        drv_uart_putchar(buffer[--i]);
    }
}
