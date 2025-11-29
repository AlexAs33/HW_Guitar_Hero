/* ***************************************************************************************
 * P.H.2025: Interfaz HAL para la Uart
 * Proporciona la interfaz necesaria para iniciar y mostrar datos en la uart a niverl hardware
 *
 * Funciones:
 * - Iniciar la uart con un baudrate determinado
 * - Mostrar un caracter en la uart
 *
 * Autores:
 * - Pablo Plumed
 * - Alex Asensio
 *************************************************************************************** */

#ifndef HAL_UART_H
#define HAL_UART_H

//Pre:  El parámetro baudrate es mayor que 0. Los pines físicos asignados a TXD y RXD
//      son válidos para el hardware específico. El reloj del periférico UART/UARTE
//      está disponible y habilitado por la plataforma.
//Post: El periférico UART/UARTE queda configurado con el baudrate solicitado.
//      Se seleccionan correctamente los pines de transmisión y recepción.
//      Se establece el formato por defecto de comunicación (8 bits, 1 stop,
//      sin flow control ni paridad, salvo que el hardware no lo permita).
void hal_uart_init(unsigned long baudrate);

//Pre:  La UART ha sido inicializada previamente mediante hal_uart_init().
//      El parámetro 'c' contiene el byte a transmitir.
//      El hardware UART/UARTE se encuentra habilitado y sin fallos graves.
//Post: El carácter 'c' se envía completamente por el periférico UART/UARTE.
//      La función no retorna hasta que el byte ha sido aceptado por el hardware
//      para su transmisión. No modifica otras configuraciones de la UART.
void hal_uart_putchar(char c);

#endif  //HAL_UART_H
