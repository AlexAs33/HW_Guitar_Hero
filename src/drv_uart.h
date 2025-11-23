#ifndef DRV_UART_H
#define DRV_UART_H

// drv_uart_log.h o drv_uart.h

#ifdef DEBUG
  #define UART_LOG_DEBUG(msg) do { drv_uart_puts("[DEBUG] "); drv_uart_puts(msg); drv_uart_puts("\r\n"); } while(0)
#else
  #define UART_LOG_DEBUG(msg) 
#endif

#define UART_LOG_INFO(msg)  do { drv_uart_puts("[INFO] ");  drv_uart_puts(msg); drv_uart_puts("\r\n"); } while(0)
#define UART_LOG_ERROR(msg) do { drv_uart_puts("[ERROR] "); drv_uart_puts(msg); drv_uart_puts("\r\n"); } while(0)

void drv_uart_init(unsigned long baudrate);
void drv_uart_putchar(char c);
char drv_uart_getchar(void);
void drv_uart_puts(char *s);
void drv_uart_putint(int valor);
int  drv_uart_data_available(void);

#endif
