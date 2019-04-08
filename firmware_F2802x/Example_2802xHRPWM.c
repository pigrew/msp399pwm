//#############################################################################
//
//  File:   Example_2802xHRPWM.c
//
//  Title:  F2802x Device HRPWM example
//
//! \addtogroup example_list
//!  <h1>High Resolution PWM</h1>
//!
//!  This example modifies the MEP control registers to show edge displacement
//!  due to the HRPWM control extension of the respective EPwm module
//!  All EPwm1A,2A,3A,4A channels (GPIO0, GPIO2, GPIO4, GPIO6) will have fine 
//!  edge movement due to HRPWM logic
//!
//!  -# PWM Freq = SYSCLK/(period=10),
//!         ePWM1A toggle low/high with MEP control on rising edge
//!         PWM Freq = SYSCLK/(period=10),
//!         ePWM1B toggle low/high with NO HRPWM control
//!
//!  -# PWM Freq = SYSCLK/(period=20),
//!         ePWM2A toggle low/high with MEP control on rising edge
//!         PWM Freq = SYSCLK/(period=20),
//!         ePWM2B toggle low/high with NO HRPWM control
//!
//!  -# PWM Freq = SYSCLK/(period=10),
//!         ePWM3A toggle as high/low with MEP control on falling edge
//!         PWM Freq = SYSCLK/(period=10),
//!         ePWM3B toggle low/high with NO HRPWM control
//!
//!  -# PWM Freq = SYSCLK/(period=20),
//!         ePWM4A toggle as high/low with MEP control on falling edge
//!         PWM Freq = SYSCLK/(period=20),
//!         ePWM4B toggle low/high with NO HRPWM control
//!
//!  Monitor ePWM1-ePWM4 pins on an oscilloscope.
//!
//!    - ePWM1A is on GPIO0
//!    - ePWM1B is on GPIO1
//!
//!    - ePWM2A is on GPIO2
//!    - ePWM2B is on GPIO3
//!
//!    - ePWM3A is on GPIO4
//!    - ePWM3B is on GPIO5
//!
//!    - ePWM4A is on GPIO6
//!    - ePWM4B is on GPIO7
//
//
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

#include "sfo_v6.h"

//
// Included Files
//
#include "DSP28x_Project.h"         // F2802x Headerfile

#include "common/include/clk.h"
#include "common/include/flash.h"
#include "common/include/gpio.h"
#include "common/include/pie.h"
#include "common/include/pll.h"
#include "common/include/pwm.h"
#include "common/include/wdog.h"

//
// Function prototypes
//
void HRPWM1_Config();
void HRPWM2_Config(Uint16);
void HRPWM3_Config(Uint16);
void HRPWM4_Config(Uint16);

static void SetupXtal(PLL_Handle myPll);
void error (void);
//
// Global variable used by the SFO library. Result can be used for all HRPWM
// channels. This variable is also copied to HRMSTEP register by SFO() function
//
int MEP_ScaleFactor;

//
// Array of pointers to EPwm register structures:
// *ePWM[0] is defined as dummy value not used in the example
//
volatile struct EPWM_REGS *ePWM[PWM_CH] =
             {  &EPwm1Regs, &EPwm1Regs, &EPwm2Regs, &EPwm3Regs, &EPwm4Regs};

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
PWM_Handle myPwm1, myPwm2, myPwm3, myPwm4;

//
// Main
//
void main(void)
{
    CPU_Handle myCpu;
    PLL_Handle myPll;
    WDOG_Handle myWDog;
    uint8_t sfoStatus;

    //
    // Initialize all the handles needed for this application
    //
    myClk = CLK_init((void *)CLK_BASE_ADDR, sizeof(CLK_Obj));
    myCpu = CPU_init((void *)NULL, sizeof(CPU_Obj));
    myFlash = FLASH_init((void *)FLASH_BASE_ADDR, sizeof(FLASH_Obj));
    myGpio = GPIO_init((void *)GPIO_BASE_ADDR, sizeof(GPIO_Obj));
    myPie = PIE_init((void *)PIE_BASE_ADDR, sizeof(PIE_Obj));
    myPll = PLL_init((void *)PLL_BASE_ADDR, sizeof(PLL_Obj));
    myPwm1 = PWM_init((void *)PWM_ePWM1_BASE_ADDR, sizeof(PWM_Obj));
    myPwm2 = PWM_init((void *)PWM_ePWM2_BASE_ADDR, sizeof(PWM_Obj));
    myPwm3 = PWM_init((void *)PWM_ePWM3_BASE_ADDR, sizeof(PWM_Obj));
    myPwm4 = PWM_init((void *)PWM_ePWM4_BASE_ADDR, sizeof(PWM_Obj));
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
    // For this case just init GPIO pins for EPwm1, EPwm2, EPwm3, EPwm4
    //
    GPIO_setPullUp(myGpio, GPIO_Number_0, GPIO_PullUp_Disable);
    GPIO_setPullUp(myGpio, GPIO_Number_1, GPIO_PullUp_Disable);
    GPIO_setMode(myGpio, GPIO_Number_0, GPIO_0_Mode_EPWM1A);
    GPIO_setMode(myGpio, GPIO_Number_1, GPIO_1_Mode_EPWM1B);

    GPIO_setPullUp(myGpio, GPIO_Number_2, GPIO_PullUp_Disable);
    GPIO_setPullUp(myGpio, GPIO_Number_3, GPIO_PullUp_Disable);
    GPIO_setMode(myGpio, GPIO_Number_2, GPIO_2_Mode_EPWM2A);
    GPIO_setMode(myGpio, GPIO_Number_3, GPIO_3_Mode_EPWM2B);

    GPIO_setPullUp(myGpio, GPIO_Number_4, GPIO_PullUp_Disable);
    GPIO_setPullUp(myGpio, GPIO_Number_5, GPIO_PullUp_Disable);
    GPIO_setMode(myGpio, GPIO_Number_4, GPIO_4_Mode_EPWM3A);
    GPIO_setMode(myGpio, GPIO_Number_5, GPIO_5_Mode_EPWM3B);

    GPIO_setPullUp(myGpio, GPIO_Number_6, GPIO_PullUp_Disable);
    GPIO_setPullUp(myGpio, GPIO_Number_7, GPIO_PullUp_Disable);
    GPIO_setMode(myGpio, GPIO_Number_6, GPIO_6_Mode_EPWM4A);
    GPIO_setMode(myGpio, GPIO_Number_7, GPIO_7_Mode_EPWM4B);

    //
    // Setup a debug vector table and enable the PIE
    //
    PIE_setDebugIntVectorTable(myPie);
    PIE_enable(myPie);

    update =1;
    DutyFine =0;

    CLK_disableTbClockSync(myClk);

    //
    // Some useful Period vs Frequency values
    //  SYSCLKOUT =     60 MHz       40 MHz
    //
    //    Period            Frequency    Frequency
    //    1000               60 kHz       40 kHz
    //    800                75 kHz       50 kHz
    //    600                100 kHz      67 kHz
    //    500                120 kHz      80 kHz
    //    250                240 kHz      160 kHz
    //    200                300 kHz      200 kHz
    //    100                600 kHz      400 kHz
    //    50                 1.2 Mhz      800 kHz
    //    25                 2.4 Mhz      1.6 MHz
    //    20                 3.0 Mhz      2.0 MHz
    //    12                 5.0 MHz      3.3 MHz
    //    10                 6.0 MHz      4.0 MHz
    //    9                  6.7 MHz      4.4 MHz
    //    8                  7.5 MHz      5.0 MHz
    //    7                  8.6 MHz      5.7 MHz
    //    6                  10.0 MHz     6.6 MHz
    //    5                  12.0 MHz     8.0 MHz
    //
    // Enable PWM clocks

    CLK_enablePwmClock(myClk, PWM_Number_1);
    CLK_enablePwmClock(myClk, PWM_Number_2);
    CLK_enableTbClockSync(myClk);
    CLK_enableHrPwmClock(myClk);
    //
    // ePWM and HRPWM register initialization
    //
    HRPWM1_Config(0xfff0);        // ePWM1 target, Period = 10
    //HRPWM2_Config(20);        // ePWM2 target, Period = 20
    HRPWM3_Config(10);        // ePWM3 target, Period = 10
    HRPWM4_Config(20);        // ePWM4 target, Period = 20


    do
        sfoStatus = SFO();
    while (sfoStatus == 0);

    if(sfoStatus == SFO_ERROR)
        error();

    while (update ==1)
    {
        sfoStatus = SFO();

        if(sfoStatus == SFO_ERROR)
            error();

        for (i=0;i<10000;i++)
        {
            asm("   NOP");
        }
    }
}

uint16_t g_period   = 0x0050;
uint32_t g_duration = 0x00388000;
uint32_t g_ratio = 3053247871ul;


//
// HRPWM1_Config - 
//
void
HRPWM1_Config() {
    uint32_t ch;

    PWM_setPeriodLoad(myPwm1, PWM_PeriodLoad_Immediate);
    PWM_setPeriodLoad(myPwm2, PWM_PeriodLoad_Immediate);
    PWM_setPeriod(myPwm1, g_period);    // Set timer period to max
    EPwm2Regs.CMPA.all = g_duration & 0xFFF0;

    for(ch=0; ch<2; ch++) {
        PWM_Handle h;
        if(ch==0)
            h = myPwm1;
        else
            h = myPwm2;

        PWM_setPeriod(h, g_period);

        PWM_setCmpA(h, g_duration>>16);
        PWM_setCmpAHr(h, (uint8_t)(g_duration & 0xFF00));
        if(ch == 0)
            PWM_setPhase(h, 0x0000);       // Phase is 0
        else
            PWM_setPhase(h, g_period/2);       // Phase is 0
        PWM_setCount(h, 0x0000);       // Clear counter

        PWM_enableCounterLoad(h);                  // Enable phase loading

        PWM_setSyncMode(h, PWM_SyncMode_EPWMxSYNC); // Allow software-sync output?
        PWM_setHighSpeedClkDiv(h, PWM_HspClkDiv_by_1);

        PWM_setClkDiv(h, PWM_ClkDiv_by_1);


        PWM_setShadowMode_CmpA(h, PWM_ShadowMode_Shadow);
        PWM_setShadowMode_CmpB(h, PWM_ShadowMode_Shadow);
        PWM_setLoadMode_CmpA(h, PWM_LoadMode_Zero);
        PWM_setLoadMode_CmpB(h, PWM_LoadMode_Zero);

        PWM_setActionQual_Zero_PwmA(h, PWM_ActionQual_Set);
        PWM_setActionQual_CntUp_CmpA_PwmA(h, PWM_ActionQual_Clear);
        PWM_setActionQual_Zero_PwmB(h, PWM_ActionQual_Clear);
        PWM_setActionQual_CntUp_CmpB_PwmB(h, PWM_ActionQual_Set);

        //
        PWM_setHrEdgeMode(h, PWM_HrEdgeMode_Falling);

        PWM_setHrControlMode(h, PWM_HrControlMode_Duty);
        PWM_setHrShadowMode(h, PWM_HrShadowMode_CTR_EQ_0);
        PWM_setCounterMode(h, PWM_CounterMode_Up);  // Count up
    }
    PWM_forceSync(myPwm1);
}

//
// HRPWM2_Config - 
//
void
HRPWM2_Config(Uint16 period)
{
    CLK_enablePwmClock(myClk, PWM_Number_2);

    //
    // ePWM2 register configuration with HRPWM
    // ePWM2A toggle low/high with MEP control on Rising edge
    //
    PWM_setPeriodLoad(myPwm2, PWM_PeriodLoad_Immediate);
    PWM_setPeriod(myPwm2, g_period);     // Set timer period
    PWM_setCmpA(myPwm2, (g_period/2 + g_duration>>16) % g_period);
    PWM_setCmpB(myPwm2,  g_period/2);
    PWM_setCmpAHr(myPwm2, (1 << 8));
    PWM_setPhase(myPwm2, 0x0000);        // Phase is 0
    PWM_setCount(myPwm2, 0x0000);        // Clear counter

    PWM_setCounterMode(myPwm2, PWM_CounterMode_Up);   // Count up
    PWM_disableCounterLoad(myPwm2);                   // Disable phase loading
    PWM_setSyncMode(myPwm2, PWM_SyncMode_Disable);
    
    //
    // Clock ratio to SYSCLKOUT
    //
    PWM_setHighSpeedClkDiv(myPwm2, PWM_HspClkDiv_by_1);
    
    PWM_setClkDiv(myPwm2, PWM_ClkDiv_by_1);

    //
    // Load registers every ZERO
    //
    PWM_setShadowMode_CmpA(myPwm2, PWM_ShadowMode_Shadow);
    PWM_setShadowMode_CmpB(myPwm2, PWM_ShadowMode_Shadow);
    PWM_setLoadMode_CmpA(myPwm2, PWM_LoadMode_Zero);
    PWM_setLoadMode_CmpB(myPwm2, PWM_LoadMode_Zero);

    PWM_setActionQual_Zero_PwmA(myPwm2, PWM_ActionQual_Disabled);
    PWM_setActionQual_CntUp_CmpA_PwmA(myPwm2, PWM_ActionQual_Clear);
    PWM_setActionQual_CntUp_CmpB_PwmA(myPwm2, PWM_ActionQual_Set);
    PWM_setActionQual_Zero_PwmB(myPwm2, PWM_ActionQual_Disabled);
    PWM_setActionQual_CntUp_CmpB_PwmB(myPwm2, PWM_ActionQual_Disabled);
    //
    // MEP control on Rising edge
    //
    PWM_setHrEdgeMode(myPwm2, PWM_HrEdgeMode_Falling);
    PWM_setHrControlMode(myPwm2, PWM_HrControlMode_Duty);
    PWM_setHrShadowMode(myPwm2, PWM_HrShadowMode_CTR_EQ_0);
}

//
// HRPWM3_Config - 
//
void
HRPWM3_Config(Uint16 period)
{
    CLK_enablePwmClock(myClk, PWM_Number_3);

    //
    // ePWM3 register configuration with HRPWM
    // ePWM3A toggle high/low with MEP control on falling edge
    //
    PWM_setPeriodLoad(myPwm3, PWM_PeriodLoad_Immediate);
    PWM_setPeriod(myPwm3, period-1);    // Set timer period
    PWM_setCmpA(myPwm3, period / 2);
    PWM_setCmpAHr(myPwm3, (1 << 8));
    PWM_setCmpB(myPwm3, period / 2);
    PWM_setPhase(myPwm3, 0x0000);       // Phase is 0
    PWM_setCount(myPwm3, 0x0000);       // Clear counter

    PWM_setCounterMode(myPwm3, PWM_CounterMode_Up);   // Count up
    PWM_disableCounterLoad(myPwm3);                   // Disable phase loading
    PWM_setSyncMode(myPwm3, PWM_SyncMode_Disable);
    
    //
    // Clock ratio to SYSCLKOUT
    //
    PWM_setHighSpeedClkDiv(myPwm3, PWM_HspClkDiv_by_1); 
    
    PWM_setClkDiv(myPwm3, PWM_ClkDiv_by_1);

    //
    // Load registers every ZERO
    //
    PWM_setShadowMode_CmpA(myPwm3, PWM_ShadowMode_Shadow);
    PWM_setShadowMode_CmpB(myPwm3, PWM_ShadowMode_Shadow);
    PWM_setLoadMode_CmpA(myPwm3, PWM_LoadMode_Zero);
    PWM_setLoadMode_CmpB(myPwm3, PWM_LoadMode_Zero);

    PWM_setActionQual_Zero_PwmA(myPwm3, PWM_ActionQual_Clear);
    PWM_setActionQual_CntUp_CmpA_PwmA(myPwm3, PWM_ActionQual_Set);
    PWM_setActionQual_Zero_PwmB(myPwm3, PWM_ActionQual_Clear);
    PWM_setActionQual_CntUp_CmpB_PwmB(myPwm3, PWM_ActionQual_Set);

    //
    // MEP control on falling edge
    //
    PWM_setHrEdgeMode(myPwm3, PWM_HrEdgeMode_Falling);
    PWM_setHrControlMode(myPwm3, PWM_HrControlMode_Duty);
    PWM_setHrShadowMode(myPwm3, PWM_HrShadowMode_CTR_EQ_0);
}

//
// HRPWM4_Config - 
//
void
HRPWM4_Config(Uint16 period)
{
    CLK_enablePwmClock(myClk, PWM_Number_4);

    //
    // ePWM4 register configuration with HRPWM
    // ePWM4A toggle high/low with MEP control on falling edge
    //
    PWM_setPeriodLoad(myPwm4, PWM_PeriodLoad_Immediate);
    PWM_setPeriod(myPwm4, period-1);    // Set timer period
    PWM_setCmpA(myPwm4, period / 2);
    PWM_setCmpAHr(myPwm4, (1 << 8));
    PWM_setCmpB(myPwm4, period / 2);
    PWM_setPhase(myPwm4, 0x0000);       // Phase is 0
    PWM_setCount(myPwm4, 0x0000);       // Clear counter

    PWM_setCounterMode(myPwm4, PWM_CounterMode_Up);   // Count up
    PWM_disableCounterLoad(myPwm4);                   // Disable phase loading
    PWM_setSyncMode(myPwm4, PWM_SyncMode_Disable);
    
    //
    // Clock ratio to SYSCLKOUT
    //
    PWM_setHighSpeedClkDiv(myPwm4, PWM_HspClkDiv_by_1);
    
    PWM_setClkDiv(myPwm4, PWM_ClkDiv_by_1);

    //
    // Load registers every ZERO
    //
    PWM_setShadowMode_CmpA(myPwm4, PWM_ShadowMode_Shadow);
    PWM_setShadowMode_CmpB(myPwm4, PWM_ShadowMode_Shadow);
    PWM_setLoadMode_CmpA(myPwm4, PWM_LoadMode_Zero);
    PWM_setLoadMode_CmpB(myPwm4, PWM_LoadMode_Zero);

    PWM_setActionQual_Zero_PwmA(myPwm4, PWM_ActionQual_Clear);
    PWM_setActionQual_CntUp_CmpA_PwmA(myPwm4, PWM_ActionQual_Set);
    PWM_setActionQual_Zero_PwmB(myPwm4, PWM_ActionQual_Clear);
    PWM_setActionQual_CntUp_CmpB_PwmB(myPwm4, PWM_ActionQual_Set);

    //
    // MEP control on falling edge
    //
    PWM_setHrEdgeMode(myPwm4, PWM_HrEdgeMode_Falling);
    PWM_setHrControlMode(myPwm4, PWM_HrControlMode_Duty);
    PWM_setHrShadowMode(myPwm4, PWM_HrShadowMode_CTR_EQ_0);


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
    PLL_setup(myPll, PLL_Multiplier_8, PLL_DivideSelect_ClkIn_by_4);

}

void
error (void)
{
    ESTOP0;     // Stop here and handle error
}

//
// End of File
//

