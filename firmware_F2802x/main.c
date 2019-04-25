//#############################################################################
// $TI Release: F2802x Support Library v3.02.00.00 $
// $Release Date: Thu Oct 18 15:45:37 CDT 2018 $
// $Copyright:
// Copyright (C) 2009-2018 Texas Instruments Incorporated - http://www.ti.com/
//
// Redistribution and use in source and binary forms, with or without 
// modification, are permitted provided that the following conditions 
// are met:
// 
//   Redistributions of source code must retain the above copyright 
//   notice, this list of conditions and the following disclaimer.
// 
//   Redistributions in binary form must reproduce the above copyright
//   notice, this list of conditions and the following disclaimer in the 
//   documentation and/or other materials provided with the   
//   distribution.
// 
//   Neither the name of Texas Instruments Incorporated nor the names of
//   its contributors may be used to endorse or promote products derived
//   from this software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// $
//#############################################################################


//
// Included Files
//
#include "F2802x_Device.h"     // F2802x Headerfile Include File
#include "main.h"
#include "pwm.h"
#include "systick.h"
#include "cmd.h"
#include "asmfuncs.h"

#include "common/include/clk.h"
#include "common/include/flash.h"
#include "common/include/gpio.h"
#include "common/include/pie.h"
#include "common/include/pll.h"
#include "common/include/pwm.h"
#include "common/include/wdog.h"

#include "uart.h"
#include "sfo_v6.h"
//
// Function prototypes
//
void HRPWM1_Config();
void HRPWM2_Config(Uint16);
void HRPWM3_Config(Uint16);
void HRPWM4_Config(Uint16);

static void SetupXtal(PLL_Handle myPll);

//
// General System nets - Useful for debug
//
Uint16 i, j, DutyFine, n, update;
Uint32 temp;

//
// Globals
//
CLK_Handle myClk;
FLASH_Handle myFlash;
GPIO_Handle myGpio;
PIE_Handle myPie;

//
// Main
//
void main(void)
{
    CPU_Handle myCpu;
    PLL_Handle myPll;
    WDOG_Handle myWDog;
    uint16_t lastTick = 0;

    //
    // Initialize all the handles needed for this application
    //
    myClk = CLK_init((void *)CLK_BASE_ADDR, sizeof(CLK_Obj));
    myCpu = CPU_init((void *)NULL, sizeof(CPU_Obj));
    myFlash = FLASH_init((void *)FLASH_BASE_ADDR, sizeof(FLASH_Obj));
    myGpio = GPIO_init((void *)GPIO_BASE_ADDR, sizeof(GPIO_Obj));
    myPie = PIE_init((void *)PIE_BASE_ADDR, sizeof(PIE_Obj));
    myPll = PLL_init((void *)PLL_BASE_ADDR, sizeof(PLL_Obj));
    myWDog = WDOG_init((void *)WDOG_BASE_ADDR, sizeof(WDOG_Obj));

    //
    // Perform basic system initialization
    //

    WDOG_disable(myWDog);

    // Load factory calibration coefficients (through ROM function)
    CLK_enableAdcClock(myClk);
    (*Device_cal)();
    CLK_disableAdcClock(myClk);

    SetupXtal(myPll);
    //
    // Disable the PIE and all interrupts
    //
    PIE_disable(myPie);
    PIE_disableAllInts(myPie);
    CPU_disableGlobalInts(myCpu);
    CPU_clearIntFlags(myCpu);

    //
    // If running from flash copy RAM only functions to RAM
    //
#ifdef _FLASH
    memcpy(&RamfuncsRunStart, &RamfuncsLoadStart, (size_t)&RamfuncsLoadSize);
#endif

    //
    // Setup a debug vector table and enable the PIE
    //
    PIE_setDebugIntVectorTable(myPie);
    PIE_enable(myPie);

    update =1;
    DutyFine =0;

    CLK_enableGpioInputClock(myClk);
    //
    // ePWM and HRPWM register initialization
    //
    pwm_init(myClk, myGpio);        // ePWM1 target, Period = 10

    uart_init();
    systick_init(myGpio);
    // Finally, enable interrupts?
    CPU_enableInt(myCpu,  CPU_IntNumber_9); // SCI interrupts
    CPU_enableGlobalInts(myCpu);

    // Debug pin
    ENABLE_PROTECTED_REGISTER_WRITE_MODE;
    GpioCtrlRegs.GPAMUX1.bit.GPIO1 = GPIO_1_Mode_GeneralPurpose;
    GpioCtrlRegs.GPADIR.bit.GPIO1 = 1; // SET as OUTPUT
    GpioCtrlRegs.GPAMUX1.bit.GPIO3 = GPIO_3_Mode_GeneralPurpose;
    GpioCtrlRegs.GPADIR.bit.GPIO3 = 1; // SET as OUTPUT

    SysCtrlRegs.XCLK.bit.XCLKOUTDIV = 3; // Disable SYSCLKOUT
    //GpioCtrlRegs.GPAMUX2.bit.GPIO18 = GPIO_18_Mode_XCLKOUT;
    //GpioCtrlRegs.GPADIR.bit.GPIO18 = 1; // SET as OUTPUT

    // Usused pins as input (pull-up): GPIO 0,1,2,3,5,7,12,16,17
    // But, >=12 default to this, anyway.
    GpioCtrlRegs.GPAPUD.all &= ~((1<<0) | (1<<1) | (1<<2) | (1<<3) | (1<<5) | (1<<7));
    // JTAG pins 35,36,37,38 also get pull-ups (for when JTAG is disabled)
    // But, this is also default.

    DISABLE_PROTECTED_REGISTER_WRITE_MODE;

    uart_write("C399PWM\n", 8);
    while (update ==1)
    {
        if(lastTick != systick_get()) {
            pwm_tick();
            lastTick = systick_get();
        }
        if(cmd_available())
            processCmds();

        asm_idle(); // Wake at either interrupt (100 Hz systick, UART, or timer)
    }
}

static void SetupXtal(PLL_Handle myPll) {
    // enable the crystal oscillator
    CLK_enableCrystalOsc(myClk);

    // Delay 1ms before converting ADC channels
   // usDelay(10000);

    // disable the external clock in
    CLK_disableClkIn(myClk);

    // set the oscillator source
    CLK_setOscSrc(myClk,CLK_OscSrc_External);

    // Switch from INTOSC1 to INTOSC2/ext clk
    CLK_setOsc2Src(myClk, CLK_Osc2Src_External);

    // enable internal oscillator 1
    CLK_enableOsc1(myClk);

    // disable oscillator 2
    CLK_disableOsc2(myClk);

    // Setup the PLL for x12 /2 which will yield 60Mhz = 10Mhz * 12 / 2
    //
    PLL_setup(myPll, PLL_Multiplier_12, PLL_DivideSelect_ClkIn_by_4);

}

void
error (void)
{
    asm_estop0();     // Stop here and handle error
}

//
// End of File
//

