/*
 * systick.c
 *
 *  Created on: Mar 23, 2019
 *      Author: nconrad
 */
#include <stdlib.h>
#include <stdint.h>
#include <msp430.h>

#include "main.h"

// Uses timer A0 to generate 1 kHz tick

volatile uint16_t systick;

void systick_init() {
    TA0CTL |= TASSEL__ACLK | ID__4; // ACLK / 4
    TA0EX0 |= TAIDEX_4; // divide by 5

    TA0CCR0 = ACLK_RATE/TICK_RATE/(4*5); // period
    TA0CCTL0 |= CCIE; // Enable CC0 interrupt

    TA0CTL |= MC__UP; // Start up-count mode, with interrupts enabled
}

uint16_t systick_get() {
    return systick;
}

#pragma vector=TIMER0_A0_VECTOR
__interrupt
void TIMER0_A0_ISR(void)
{
    systick++;
    __bic_SR_register_on_exit (LPM0_bits);
}
