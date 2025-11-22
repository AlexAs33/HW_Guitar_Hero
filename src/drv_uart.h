#ifndef DRV_UART_H
#define DRV_UART_H

void drv_uart_init(unsigned long baudrate);
void drv_uart_putchar(char c);
char drv_uart_getchar(void);
void drv_uart_puts(char *s);
void drv_uart_putint(int valor);
int  drv_uart_data_available(void);

#endif
