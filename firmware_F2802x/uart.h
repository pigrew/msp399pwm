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

void uart_init(CLK_Handle myClk, GPIO_Handle myGpio, PIE_Handle pieHandle);
uint16_t uart_write(uint16_t *data, size_t len);


#endif /* UART_H_ */
