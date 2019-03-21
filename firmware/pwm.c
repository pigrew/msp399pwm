/*
 * pwm.c
 *
 *  Created on: Mar 21, 2019
 *      Author: nconrad
 */


#include "driverlib.h"

#include "pwm.h"
#include "main.h"

void pwm_init() {
#ifdef START_XTAL
    Timer_D_initHighResGeneratorInRegulatedModeParam td_reg_params =
        {
         .clockingMode = TIMER_D_CLOCKINGMODE_HIRES_LOCAL_CLOCK,
         .clockSource = TIMER_D_CLOCKSOURCE_SMCLK,
         .clockSourceDivider = TIMER_D_CLOCKSOURCE_DIVIDER_1,
         .highResClockDivider = TIMER_D_HIGHRES_CLK_DIVIDER_1,
         .highResClockMultiplyFactor = TIMER_D_HIGHRES_CLK_MULTIPLY_FACTOR_16x
    };

    Timer_D_initHighResGeneratorInRegulatedMode(TIMER_D0_BASE, &td_reg_params);
    TD0HCTL0 &= ~(TDHEAEN); // Disable the enhanced-accuracy mode (which increases short-term jitter)
#else

    Timer_D_initHighResGeneratorInFreeRunningMode(TIMER_D0_BASE, TIMER_D_HIGHRES_256MHZ);

#endif

    // CCR0 controls the period

    //TD0HCTL1 = CALTDH0CTL1_256;                 // Read the 256Mhz TimerD TLV Data
    TD0CTL1 |= TDCLKM_1;                      // Select Hi-res local clock
    //TD0HCTL0 = TDHEN + TDHM_1;                // CALEN=0 => free running mode; enable Hi-res mode
    //                                          // THDMx = 01 => 16x
    // P2.4 <= TD0.0 PWMA
    // TD0.0 and TD0.1 pins
    GPIO_setAsPeripheralModuleFunctionOutputPin(
                      GPIO_PORT_P2,
                      GPIO_PIN4 | GPIO_PIN5 | GPIO_PIN6
                      );

    /*Timer_D_enableHighResInterrupt(TIMER_D0_BASE,
                  TIMER_D_HIGH_RES_FREQUENCY_LOCK
                  );*/
    //Timer_D_enableCaptureCompareInterrupt(TIMER_D0_BASE, TIMER_D_CAPTURECOMPARE_REGISTER_0);

    // Configure TimerD in Hi-Res Regulated Mode
    // TDCLK=SMCLK=12MHz=Hi-Res input clk select
    // Configure the CCRx blocks
    // Configure the CCRx blocks

    // Setup TD0CCR0: (interrupt enable) | (reset-set mode)  | (latch when timer0 goes to 0) |
    TD0CCTL0 =      OUTMOD_4 | CCIE ;//            | OUTMOD_4          | CLLD_1; // Toggle, just for fun
    TD0CCTL1 =        OUTMOD_7; // 0                | OUTMOD_7          | CLLD_1;
    TD0CCTL2 = OUTMOD_5;
    // To get divisor, take CCR0, round down to 4, add 4.
    TD0CCR0 = PERIOD;
    TD0CCR1 = WMIN;

    TD0CTL1 |= TDCLKM__HIGHRES;
    TD0CTL1 &= ~TD2CMB; // don't combine CCR1 and CCR2


    // Setup TD0: (interrupt enable) | (Use SM CLK)  |  16-bit
    //TD0CTL0     |= TDIE              | TDSSEL__SMCLK | CNTL__16;

    TD0CTL0 |= MC_1 | TDCLR;                  // up-mode, clear TDR, Start timer
    //TD0CTL0 |= TDIE;
}
