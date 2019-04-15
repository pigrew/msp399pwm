/*
 * uart.h
 *
 *  Created on: Apr 8, 2019
 *      Author: nconrad
 */

#ifndef UART_H_
#define UART_H_

#include "ring_buffer.h"

#define RXBUF_SIZE 20
extern volatile size_t rxBufLen;
extern volatile RB_ELM_TYPE rxbuf[RXBUF_SIZE + 1]; // room for NULL byte
extern volatile bool cmdComplete;

void uart_init();
uint16_t uart_write(char *data, size_t len);
void u16hex(uint32_t value, char* result, uint16_t bits);
uint16_t u16dec(uint32_t value, char* result);


#endif /* UART_H_ */
