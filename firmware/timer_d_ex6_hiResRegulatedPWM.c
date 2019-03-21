#include "driverlib.h"
#include "uart.h"
#include "pwm.h"
#include "main.h"

// PWMA is TD0.0
volatile uint16_t cv = WMIN;

void main(void)
{
    WDT_A_hold(WDT_A_BASE);

    // Configure XT1
    // Port select XT1
    // COREV_0 <= 12M
    // COREV_1 <= 16M
    // COREV_2 <= 20M
    // COREV_3 <= 25.5M

    PMM_setVCore (PMMCOREV_3);

#ifdef START_XTAL
    // XTAL pins
    GPIO_setAsPeripheralModuleFunctionInputPin(
          		  GPIO_PORT_PJ,
          		  GPIO_PIN4 + GPIO_PIN5
          		  );

    UCS_turnOnHFXT1(UCS_XT1_DRIVE_1);


    // Initialize DCO to 12MHz
    UCS_initFLLSettle(24000,
                  3);

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

#endif


    uart_init();
    pwm_init();

    __bis_SR_register(LPM0_bits + GIE);       // Enter LPM0
    __no_operation();                         // For debugger
}

#pragma vector=TIMER0_D0_VECTOR
__interrupt
void TIMER0_D0_ISR(void)
{
   /* cv = cv + 1;
    if(cv > WMAX)
       cv = WMIN;
    TD0CCR1 = cv;*/
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


