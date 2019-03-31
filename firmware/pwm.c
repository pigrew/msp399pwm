/*
 * pwm.c
 *
 *  Created on: Mar 21, 2019
 *      Author: nconrad
 */


#include "driverlib.h"

#include "pwm.h"
#include "main.h"

#define DISCIPLINED

// PWMA is TD0.0
static uint16_t g_period; // period as set over UART
static uint32_t g_ratio; // ratio as set over UART

static volatile uint16_t pwmA_base;     // touched by ISR and user code,
static volatile uint32_t pwmA_fraction; // touched by ISR and user code, must disable interrupts before changing.

static void pwm_applyRatio();

void pwm_init() {
    g_period = 0x9600;    // 10 kHz
    g_ratio = 3053247871ul;
#ifdef DISCIPLINED
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
                      GPIO_PIN4 | GPIO_PIN5 //| GPIO_PIN6
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
    TD0CCTL1 =        OUTMOD_7 | CLLD_1; // 0                | OUTMOD_7          | CLLD_1;
    TD0CCTL2 = OUTMOD_5;
    // To get divisor, take CCR0, round down to 4, add 4.
    pwm_setPeriod(g_period); // will set both period and ratio

    TD0CTL1 |= TDCLKM__HIGHRES;
    TD0CTL1 &= ~TD2CMB; // don't combine CCR1 and CCR2


    // Setup TD0: (interrupt enable) | (Use SM CLK)  |  16-bit
    //TD0CTL0     |= TDIE              | TDSSEL__SMCLK | CNTL__16;

    TD0CTL0 |= MC_1 | TDCLR;                  // up-mode, clear TDR, Start timer
    //TD0CTL0 |= TDIE;
}

void pwm_setPeriod(uint16_t period) {
    if(period < 16)
        period = 16; // minimum for PWM generator
    period = period & ~(0x0007); // last few bits are ignored by timer

    // CCR1 may not immediately load, so try to load it manually?
    // Recalculate and reapply ratio
    if(period < g_period) { // reducing period
        pwm_applyRatio(period);
        TD0CCR0 = period-1; // must subtract 1 to get proper period!
    } else {
        TD0CCR0 = period-1; // must subtract 1 to get proper period!
        pwm_applyRatio(period);
    }
    g_period = period;
}
#define MAX_32_D ((double)4294967295.0)


__attribute__((ramfunc))
static void pwm_applyRatio(uint16_t period) {
    // float is only 24-bit, so we need double-precision if using floating-point for math.
    double r = ((double)g_ratio)/MAX_32_D;
    double high = r*period;
    pwmA_base = (uint16_t)high;
    uint32_t actualBase = (((double)pwmA_base)/ ((double)period))*MAX_32_D;

    r = ((double)(g_ratio - actualBase)) * ((double)period); // remainder

    __disable_interrupt();
    pwmA_fraction = ((uint32_t)(r))>>2;  // shift right two bits to make room for accumulators in delta-sigma method
    __enable_interrupt();

    TD0CCR1 = pwmA_base;
    //TD0CL1 = pwmA_base;

}

void pwm_setRatio(uint32_t ratio) {

    g_ratio = ratio;
    pwm_applyRatio(g_period);
}

#define HIGH_BYTE(x) (*(((uint8_t*)&x)+3))
#define HIGH_WORD(x) (*(((uint16_t*)&x)+1))
// delta-sigma modulator, see http://www.ti.com/lit/an/slyt076/slyt076.pdf
#define DS_N (30)
#pragma vector=TIMER0_D0_VECTOR
__attribute__((ramfunc))
__interrupt
void TIMER0_D0_ISR(void) { // 3.13us
    PAOUT_H |= (1 << (6)); // clear p2.6
    static uint32_t pwmA_fraction_sigma = (1<<31);    // only touched by ISR, so no need for volatile.

    uint32_t delta = 0;
    if (HIGH_WORD(pwmA_fraction_sigma) & (0x8000)) // if highest bit set?
        delta = (3ul << DS_N);
    uint32_t delta_out = pwmA_fraction + delta;
    uint32_t sigma_out = delta_out + pwmA_fraction_sigma;
    pwmA_fraction_sigma = sigma_out;
    uint16_t x = 0;
    if(HIGH_WORD(pwmA_fraction_sigma) & (0x8000)) // if highest bit set?
        x=1;
    TD0CCR1 = x + pwmA_base;

    PAOUT_H &= ~(1 << (6)); // clear p2.6
}

// Timer0_D1 Interrupt Vector (TDIV) handler
#pragma vector=TIMER0_D1_VECTOR
__interrupt
void TIMER0_D1_ISR(void)
{
    switch(__even_in_range(TD0IV,30))
    {
    case  0: break;                          // No interrupt
    case  2: break;                          // CCR1 not used
    case  4: break;                          // CCR2 not used
    case  6: break;                          // reserved
    case  8: break;                          // reserved
    case 10: break;                          // reserved
    case 12: break;                          // reserved
    case 14: break;
    case 16: break;
    case 18:                                 // Clock fail low
      while(1);                              // Input ref clock freq too low; trap here
    case 20:                                 // Clock fail high
      while(1);                              // Input ref clock freq too high; trap here
    case 22:                                 // Hi-res freq locked
      // Hi-Res freq locked; now configure ports to output PWMs at TD0.0/1/2
      // P1.6,7 option select
      // P1.6,7 output
      // P2.0 options select
      // P2.0 output
      /*GPIO_setAsPeripheralModuleFunctionOutputPin(
                  GPIO_PORT_P1,
                  GPIO_PIN6 + GPIO_PIN7
                  );

      GPIO_setAsPeripheralModuleFunctionOutputPin(
                  GPIO_PORT_P2,
                  GPIO_PIN0
                  );*/
      break;
    case 24: break;                          // Hi-res freq unlocked
    case 26: break;                          // reserved
    case 28: break;                          // reserved
    case 30: break;                          // reserved
    default: break;
    }
}


