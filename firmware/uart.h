/*
 * uart.h
 *
 *  Created on: Mar 20, 2019
 *      Author: nconrad
 */

#ifndef UART_H_
#define UART_H_

#include <stdlib.h>

void uart_init();
uint8_t uart_putc(uint8_t c);
uint8_t uart_write(uint8_t *data, size_t len);

#endif /* UART_H_ */
