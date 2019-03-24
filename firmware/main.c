#include "driverlib.h"
#include "uart.h"
#include "pwm.h"
#include "main.h"
#include "cmd.h"
#include "tmp411.h"
#include "systick.h"
#include <string.h>

// globals!
static bool g_tempReport = false;
static uint16_t g_lastTempTick = 0;


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

    // Initialize DCO to 24MHz
    UCS_initFLLSettle(DCO_RATE/1000ul, DCO_RATE/FLLREF_RATE);
#else
    // FLLREFDIV = 1
    // UCSCLT1: DCORSEL=6 (range)
    // UCSCLT2: FLLD=000b/Div1 ; FLLN=3
    // UCSCTL3: SELREF=000b/XT1CLK; FLLREFDIV=000/Div1
    UCS_initClockSignal(UCS_FLLREF, UCS_REFOCLK_SELECT, UCS_CLOCK_DIVIDER_2);

    // Initialize DCO to 24MHz
    UCS_initFLLSettle(DCO_RATE/1000ul, DCO_RATE/32768);
#endif

    UCS_initClockSignal(UCS_MCLK, UCS_DCOCLKDIV_SELECT, UCS_CLOCK_DIVIDER_2);
    UCS_initClockSignal(UCS_SMCLK, UCS_DCOCLK_SELECT, UCS_CLOCK_DIVIDER_1);
    UCS_initClockSignal(UCS_ACLK, UCS_DCOCLKDIV_SELECT, UCS_CLOCK_DIVIDER_8); // 1.5 MHz for USCI


    uart_init();
    pwm_init();
    tmp411_init();
    systick_init();

    g_lastTempTick = 0;

    // Enable interrupts!
    __enable_interrupt();
    while(true) {
        uint16_t now = systick_get();
        processCmds();

        // get temperature every second
        if((now-g_lastTempTick) > 1000) {
            uint8_t str[8];
            uint16_t lt = tmp411_getLocal();
            uint16_t rt = tmp411_getRemote();
            if(g_tempReport) {
                uart_write("T",1);
                u16hex(lt,(char*)str,16);
                uart_write(str, 4);
                uart_write(",", 1);
                u16hex(rt,(char*)str,16);
                uart_write(str, 4);
                uart_write("\n",1);
            }

            g_lastTempTick += 1000;
            //  !(in next 1000 ticks )                && more than 1000 ago
            if( (g_lastTempTick + 1000 - now <= 1000) && ((now - g_lastTempTick) > 1000)) // too long ago, reset timer.
                g_lastTempTick = now;
        }
        __bis_SR_register(LPM0_bits);       // Enter LPM0
        __no_operation();                         // For debugger
    }
}

void main_set_tempReport(bool enabled) {
    g_lastTempTick = systick_get();
    g_tempReport = enabled;
}
