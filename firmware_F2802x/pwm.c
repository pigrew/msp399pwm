/*
 * pwm.c
 *
 *  Created on: Apr 8, 2019
 *      Author: nconrad
 */
#include "sfo_v6.h"
//
// Included Files
//
#include "F2802x_Device.h"     // F2802x Headerfile Include File
#include "common/include/clk.h"
#include "common/include/gpio.h"
#include "common/include/pwm.h"

#include "main.h"
#include "systick.h"


PWM_Handle myPwm1, myPwm2;
//
// Array of pointers to EPwm register structures:
// *ePWM[0] is defined as dummy value not used in the example
//
//
// Equal # of HRPWM channels PLUS 1
// i.e. PWM_CH is 9 for 8 channels, 7 for 6 channels, etc.
//
#define PWM_CH  3
volatile struct EPWM_REGS *ePWM[PWM_CH] =
             {  &EPwm1Regs, &EPwm1Regs, &EPwm2Regs};
int MEP_ScaleFactor;
#pragma DATA_ALIGN(MEP_ScaleFactor,2); // Align for MAC usage

static uint16_t g_period   = 0xFFFE;
static uint32_t g_duration = 0xB5FCDD00;
static uint32_t g_ratio = 0xB5FCDD7F;
#pragma DATA_ALIGN(g_period,2); // Align for MAC usage
#pragma DATA_ALIGN(g_ratio,2); // Align for MAC usage

static void pwm_applyRatio(uint16_t period);

void pwm_init() {
    uint32_t ch;
    int sfoStatus;

    myPwm1 = PWM_init((void *)PWM_ePWM1_BASE_ADDR, sizeof(PWM_Obj));
    myPwm2 = PWM_init((void *)PWM_ePWM2_BASE_ADDR, sizeof(PWM_Obj));

    ENABLE_PROTECTED_REGISTER_WRITE_MODE;
    GpioCtrlRegs.GPAPUD.bit.GPIO0 = GPIO_PullUp_Disable;
    GpioCtrlRegs.GPAPUD.bit.GPIO2 = GPIO_PullUp_Disable;
    GpioCtrlRegs.GPAMUX1.bit.GPIO0 = GPIO_0_Mode_EPWM1A;
    GpioCtrlRegs.GPAMUX1.bit.GPIO2 = GPIO_2_Mode_EPWM2A;
    DISABLE_PROTECTED_REGISTER_WRITE_MODE;


    CLK_enableTbClockSync(myClk);
    CLK_enablePwmClock(myClk, PWM_Number_1);
    CLK_enablePwmClock(myClk, PWM_Number_2);
    CLK_enableHrPwmClock(myClk);

    PWM_setPeriodLoad(myPwm1, PWM_PeriodLoad_Immediate);
    PWM_setPeriodLoad(myPwm2, PWM_PeriodLoad_Immediate);
    PWM_setPeriod(myPwm1, g_period);    // Set timer period to max
    EPwm2Regs.CMPA.all = g_duration & 0xFFF0;

    do
        sfoStatus = SFO();
    while (sfoStatus == 0);

    if(sfoStatus == SFO_ERROR)
        error();

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
        PWM_disableAutoConvert(h); // We know better than the hardware when MEP changes. :)
        PWM_setCounterMode(h, PWM_CounterMode_Up);  // Count up
    }
    EPwm1Regs.TBCTL.bit.FREE_SOFT = 3; // Continue PWM when debugger pauses code
    EPwm2Regs.TBCTL.bit.FREE_SOFT = 3; // Continue PWM when debugger pauses code


    PWM_forceSync(myPwm1);
    pwm_applyRatio(g_period);
}

void pwm_tick() {
    if((systick_get() & 0x007F)== 0) {
        int sfoStatus = SFO();

        if(sfoStatus == SFO_ERROR)
            error();
    }
}
void pwm_setRatio(uint32_t ratio) {

    g_ratio = ratio;
    pwm_applyRatio(g_period);
}
// Manual says MEP is number per clock cycle;
#define MAX_32_D ((double)4294967295.0)
static void pwm_applyRatio(uint16_t period) {
    uint32_t high0;
    high0 = ((uint32_t)(uint16_t)period) * ((uint32_t)(uint16_t)(MEP_ScaleFactor));
    uint64_t high = (((uint64_t)high0) * ((uint64_t)g_ratio)) << 1;
    high += (1ul<<31)-1u;
    uint32_t count = high>>32;
    uint16_t CMP1 = (count / ((uint16_t)MEP_ScaleFactor))>>1;
    uint16_t CMP2 = CMP1;

    count -= (((uint32_t)CMP1)*((uint32_t)MEP_ScaleFactor))<<1;
    uint16_t mep1 = count/2;
    uint16_t mep2 = count-mep1;
    if(mep2 == MEP_ScaleFactor) {
        CMP2 = CMP2 + 1;
        mep2 = 0;
    }
    EPwm1Regs.CMPA.all = (((uint32_t)CMP1)<<16) | (mep1<<8);
    EPwm2Regs.CMPA.all = (((uint32_t)CMP2)<<16) | (mep2<<8);

}
void pwm_applyWhole(uint16_t x) {
    EPwm1Regs.CMPA.half.CMPA = x;
    EPwm2Regs.CMPA.half.CMPA = x;
}
void pwm_applyMEP(uint16_t x) {
    uint16_t a = x>>1;
    EPwm1Regs.CMPA.half.CMPAHR = a<<8;
    EPwm2Regs.CMPA.half.CMPAHR = (x-a)<<8;
}
