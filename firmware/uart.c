/*
 * uart.c
 *
 *  Created on: Mar 20, 2019
 *      Author: nconrad
 */

#include <stdlib.h>
#include "driverlib.h"
#include "ring_buffer.h"

#define RXBUF_SIZE 20
// TB RB size must be power of 2
#define TXBUF_SIZE 32

volatile bool cmdComplete;
volatile size_t rxBufLen = 0;
uint8_t rxbuf[RXBUF_SIZE];

volatile bool tx_active = false;
uint8_t txbuf[TXBUF_SIZE];
struct ring_buffer tx_rb;

void uart_init() {
    GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P1, GPIO_PIN1);
    GPIO_setAsPeripheralModuleFunctionInputPin (GPIO_PORT_P1, GPIO_PIN2);
    USCI_A_UART_initParam  params =
    { // 24 MHz, 38400, 8N1
     .selectClockSource = USCI_A_UART_CLOCKSOURCE_SMCLK,
     .clockPrescalar = 39,
     .firstModReg = 1,
     .secondModReg = 0,
     .overSampling = 1,
     .parity = USCI_A_UART_NO_PARITY,
     .msborLsbFirst = USCI_A_UART_LSB_FIRST,
     .numberofStopBits = USCI_A_UART_ONE_STOP_BIT,
     .uartMode = USCI_A_UART_MODE
    };

    if ( STATUS_FAIL == USCI_A_UART_init ( USCI_A0_BASE, &params )) {
        return;
    }
    USCI_A_UART_enableInterrupt (USCI_A0_BASE, USCI_A_UART_RECEIVE_INTERRUPT);
    USCI_A_UART_enable(USCI_A0_BASE);
    tx_rb.head = 0;
    tx_rb.tail = 0;
    tx_rb.buf = txbuf;
    tx_rb.n_elem = TXBUF_SIZE;
}
// 0 for success
uint8_t uart_putc(uint8_t c) {
    uint8_t r = rb_put(&tx_rb, c);
    if(!tx_active) { // data can be lost here, if the buffer overflowed.
        tx_active = true;
        if(0 == rb_get(&tx_rb, &c))
            USCI_A_UART_transmitData(USCI_A0_BASE, c);
    }
    return r;
}
// 0 on success
uint8_t uart_write(uint8_t *data, size_t len) {
    uint8_t r = 0;
    uint8_t i;
    for(i=0; i<len; i++) {
        if(rb_put(&tx_rb, data[i])) {
            r = 1;
            break; // give up on overflow
        }
    }
    if(!tx_active) { // data can be lost here, if the buffer overflowed.
        uint8_t c;
        tx_active = true;
        if(0 == rb_get(&tx_rb, &c))
            USCI_A_UART_transmitData(USCI_A0_BASE, c);
    }
    return r;
}

#pragma vector=USCI_A0_VECTOR
__interrupt
void USCI_A0_ISR(void) {
    uint8_t d, r;
    switch(__even_in_range(UCA0IV,4))  {
    // Vector 2 - RXIFG
    case USCI_UCRXIFG:
        if(cmdComplete) // Can't receive before buffer handled!
            break;

        d = USCI_A_UART_receiveData(USCI_A0_BASE);

        if(rxBufLen < RXBUF_SIZE)
            rxbuf[rxBufLen++] = d;
        else
            rxBufLen = 0; /* Overflow, clear buffer */
        break;
    case USCI_UCTXIFG:
        // tx_active must be true
        r = rb_get(&tx_rb, &d);
        if(r == 0) { // success
            USCI_A_UART_transmitData(USCI_A0_BASE, d);
        } else {
            tx_active = false;
        }
        break;
    default:
        break;
    }
}

