#ifndef HAL_UART_H
#define HAL_UART_H

void hal_uart_init(unsigned long baudrate);
void hal_uart_putchar(char c);
char hal_uart_getchar(void);
int  hal_uart_data_available(void);

#endif
