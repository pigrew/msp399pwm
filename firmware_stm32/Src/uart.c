/*
 * uart.c
 *
 *  Created on: Apr 2, 2019
 *      Author: nconrad
 */



#include <stdlib.h>
#include <stdbool.h>
#include "stm32f3xx_hal.h"
#include "ring_buffer.h"
#include "uart.h"

#define TXBUF_BITS 5

volatile bool cmdComplete = false;
volatile size_t rxBufLen = 0;
volatile uint8_t rxbuf[RXBUF_SIZE+1]; // leave a spot for a null terminator

volatile bool tx_active = false;
volatile uint8_t txbuf[1<<TXBUF_BITS];
struct ring_buffer tx_rb;

uint8_t txbuf2[1<<TXBUF_BITS]; // buffer used by HAL for transmit.

static UART_HandleTypeDef *huart_;

void uart_init(UART_HandleTypeDef* huart) {
	huart_ = huart;

    tx_rb.head = 0;
    tx_rb.tail = 0;
    tx_rb.buf = txbuf;
    tx_rb.n_bits = TXBUF_BITS;
}
uint8_t loadHALTxBufferAndTX() {

    uint8_t i=0;
    while((i < (1<<TXBUF_BITS)) && (0 == rb_get(&tx_rb, &txbuf2[i]))) {
        i++;
    }
    if(i)
    	HAL_UART_Transmit_IT(huart_, txbuf2, i);
    return i;
}
// 0 for success
uint8_t uart_putc(uint8_t c) {
    uint8_t r = rb_put(&tx_rb, c);
    if(!tx_active) { // data can be lost here, if the buffer overflowed.
        tx_active = true;
        loadHALTxBufferAndTX();
    }
    return r;
}
// 0 on success
uint8_t uart_write(uint8_t *data, size_t len) {
    uint8_t r = 0;
    uint8_t i;
    // Load ring buffer
    for(i=0; i<len; i++) {
        if(rb_put(&tx_rb, data[i])) {
            r = 1;
            break; // give up on overflow
        }
    }
    // And now start the transmit, if not transmitting
    if(!tx_active) { // data can be lost here, if the buffer overflowed.
        tx_active = true;
        loadHALTxBufferAndTX();
    }
    return r;
}
static uint8_t baseFourChar(uint8_t baseFour) {
    baseFour = baseFour & 0x0F;
    if(baseFour < 10)
        return '0' + baseFour;
    return 'A' + baseFour - 10;
}
// Convert 16-bit hex to string.
// Does not append NULL.
void u16hex(uint32_t value, char* result, uint8_t bits) {
    uint8_t i = 0;
    for( ; bits>0 ; bits-=4) {
        result[i++] = baseFourChar(value >> (bits-4));
    }
    return;
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if(loadHALTxBufferAndTX()) {
	} else {
        tx_active = false;
	}
}
/*
#pragma vector=USCI_A0_VECTOR
__attribute__((ramfunc))
__interrupt
void USCI_A0_ISR(void) {
    uint8_t d, r;

    //PAOUT_H |= (1 << (6)); // set p2.6
    switch(__even_in_range(UCA0IV,4))  {
    // Vector 2 - RXIFG
    case USCI_UCRXIFG:

        d =HWREG8(USCI_A0_BASE + OFS_UCAxRXBUF);
        if(cmdComplete) // disregard if previous CMD has not been handled
            return;

        if(rxBufLen < RXBUF_SIZE)
            rxbuf[rxBufLen++] = d;
        else
            rxBufLen = 0; /* Overflow, clear buffer * /

        if(d == '\n' || d == '\r') {
            if(rxBufLen == 1) {
                rxBufLen = 0;
            } else {
                // Line complete
                cmdComplete = true;
                __bic_SR_register_on_exit (LPM0_bits);
            }
        }
        break;

    default:
        break;
    }

    //PAOUT_H &= ~(1 << (6)); // clear p2.6
}*/
