/* *****************************************************************************
 * P.H.2025: Interfaz del Driver de la UART
 *
 * Este módulo proporciona una interfaz de alto nivel para la comunicación UART,
 * encapsulando la implementación de bajo nivel del HAL. Permite inicializar la
 * UART, enviar y recibir caracteres, transmitir cadenas de texto, enviar
 * números enteros y comprobar si hay datos disponibles.
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

// Pre:  El valor de baudrate debe ser válido según la configuración del sistema.
// Post: La UART queda inicializada y lista para transmitir y recibir datos.
void drv_uart_init(unsigned long baudrate);

// Pre:  La UART debe estar inicializada mediante drv_uart_init().
// Post: El carácter indicado se envía por la interfaz UART.
void drv_uart_putchar(char c);

// Pre:  La UART debe estar inicializada. Debe existir al menos un carácter disponible.
// Post: Devuelve el carácter recibido desde la UART.
char drv_uart_getchar(void);

// Pre:  La UART debe estar inicializada. El puntero 's' debe ser válido y
//       apuntar a una cadena terminada en '\0'.
// Post: La cadena completa se envía por la UART carácter a carácter.
void drv_uart_puts(char *s);

// Pre:  La UART debe estar inicializada correctamente.
//       'valor' es un entero válido (positivo, negativo o cero).
// Post: El número entero representado por 'valor' se envía por la UART
//       carácter a carácter, sin añadir salto de línea ni espacios.
void drv_uart_putint(int valor);

#endif  //DRV_UART_H
