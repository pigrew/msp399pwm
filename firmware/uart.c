/*
 * uart.c
 *
 *  Created on: Mar 20, 2019
 *      Author: nconrad
 */

#include <stdlib.h>
#include "driverlib.h"

#define RXBUF_SIZE 20
#define TXBUF_SIZE 20

size_t rxBufLen = 0;
char rxbuf[RXBUF_SIZE];

size_t txBufLen = 0;
char txbuf[TXBUF_SIZE];

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

}

#pragma vector=USCI_A0_VECTOR
__interrupt
void USCI_A0_ISR(void) {
    switch(__even_in_range(UCA0IV,4))  {
    // Vector 2 - RXIFG
    case 2:
        // Echo back RXed character, confirm TX buffer is ready first
        // USCI_A0 TX buffer ready?
        while (!USCI_A_UART_interruptStatus(USCI_A0_BASE, UCTXIFG))
            ;
        //Receive echoed data
        receivedData = USCI_A_UART_receiveData(USCI_A0_BASE);
        //Transmit next data
        USCI_A_UART_transmitData(USCI_A0_BASE, transmitData++);
        break;
    default:
        break;
    }
}
