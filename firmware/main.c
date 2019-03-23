#include "driverlib.h"
#include "uart.h"
#include "pwm.h"
#include "main.h"
#include "cmd.h"
#include "tmp411.h"

// globals!
bool reportTemps = false;

// PWMA is TD0.0
volatile uint16_t cv = WMIN;

// XT1 is 8MHz
// DCOCLKDIV (FLL) is XT1*3=24MHz
// SMCLK is (DCOCLKDIV/1) = 24 MHz (timer_d)
// ACLK is (XT1) = 8 MHz
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
    UCS_setExternalClockSource(8000000,0);

    UCS_turnOnHFXT1(UCS_XT1_DRIVE_2);
    // FLLREFDIV = 1
    // UCSCLT1: DCORSEL=6 (range)
    // UCSCLT2: FLLD=000b/Div1 ; FLLN=3
    // UCSCTL3: SELREF=000b/XT1CLK; FLLREFDIV=000/Div1
    UCS_initClockSignal(UCS_FLLREF, UCS_XT1CLK_SELECT, UCS_CLOCK_DIVIDER_2);

    // Initialize DCO to 12MHz
    UCS_initFLLSettle(24000,
                  6);

    UCS_initClockSignal(UCS_MCLK, UCS_DCOCLKDIV_SELECT, UCS_CLOCK_DIVIDER_2);
    UCS_initClockSignal(UCS_SMCLK, UCS_DCOCLK_SELECT, UCS_CLOCK_DIVIDER_1);
    UCS_initClockSignal(UCS_ACLK, UCS_DCOCLKDIV_SELECT, UCS_CLOCK_DIVIDER_8); // 1.5 MHz for USCI

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
    tmp411_init();
    // Enable interrupts!
    __enable_interrupt();
    while(true) {
        __delay_cycles(100000);
        processCmds();
        //uint8_t *str = "Blah\n";
        uint16_t lt = tmp411_getLocal();
        uint16_t rt = tmp411_getRemote();
        if(reportTemps) {
            uart_write((uint8_t*)&lt, 2);
            uart_write((uint8_t*)&rt, 2);
        }
    }

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



