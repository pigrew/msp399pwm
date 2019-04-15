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


//
// Array of pointers to EPwm register structures:
// *ePWM[0] is defined as dummy value not used in the example
//
//
// Equal # of HRPWM channels PLUS 1
// i.e. PWM_CH is 9 for 8 channels, 7 for 6 channels, etc.
//

volatile struct EPWM_REGS *ePWM[PWM_CH] =
             {  &EPwm1Regs, &EPwm1Regs, &EPwm2Regs, &EPwm3Regs, &EPwm4Regs};
uint16_t MEP_ScaleFactor=0;
uint16_t MEP_ScaleFactor_16;
#pragma DATA_ALIGN(MEP_ScaleFactor,2); // Align for MAC usage

static uint16_t g_period   = 0xFFFE;
//static uint32_t g_duration = 0xB5FCDD00;
static uint32_t g_ratio = 0xB5FCDD7F;
#pragma DATA_ALIGN(g_period,2); // Align for MAC usage
#pragma DATA_ALIGN(g_ratio,2); // Align for MAC usage

static void pwm_applyRatio(uint16_t period);

void pwm_init() {
    uint32_t ch;
    int sfoStatus;

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

    /*
    PWM_setPeriodLoad(myPwm1, PWM_PeriodLoad_Immediate);
    PWM_setPeriodLoad(myPwm2, PWM_PeriodLoad_Immediate);
    PWM_setPeriod(myPwm1, g_period);    // Set timer period to max
    EPwm2Regs.CMPA.all = g_duration & 0xFFF0;
     */
    do
        sfoStatus = SFO();
    while (sfoStatus == 0);

    if(sfoStatus == SFO_ERROR)
        error();

    for(ch=0; ch<2; ch++) {
        volatile struct EPWM_REGS *pwmRegs;

        if(ch==0) {
            pwmRegs = &EPwm1Regs;
        } else {
            pwmRegs = &EPwm2Regs;
        }
        pwmRegs->TBPRD = g_period;

        // Default CMPA to 50% duty cycle
        pwmRegs->CMPA.half.CMPA = g_period>>1;
        pwmRegs->CMPA.half.CMPAHR = 0;

        pwmRegs->TBPHS.half.TBPHSHR = 0x0000;
        if(ch == 0)
            pwmRegs->TBPHS.half.TBPHS = 0x0000;       // Phase is 0
        else
            pwmRegs->TBPHS.half.TBPHS = g_period>>1;       // Phase offset of 180 degrees

        pwmRegs->TBCTR = 0x0000;       // Clear counter

        pwmRegs->TBCTL.all = PWM_RunMode_FreeRun | PWM_ClkDiv_by_1 | PWM_HspClkDiv_by_1 | PWM_SyncMode_EPWMxSYNC
                | PWM_PeriodLoad_Shadow | PWM_TBCTL_PHSEN_BITS | PWM_CounterMode_Stop;

        pwmRegs->CMPCTL.bit.SHDWAMODE = CC_SHADOW;
        pwmRegs->CMPCTL.bit.SHDWBMODE = CC_SHADOW;

        pwmRegs->CMPCTL.bit.LOADAMODE = CC_CTR_ZERO;
        pwmRegs->CMPCTL.bit.LOADBMODE = CC_CTR_ZERO;

        pwmRegs->AQCTLA.bit.ZRO = AQ_SET;   // Zero
        pwmRegs->AQCTLA.bit.CAU = AQ_CLEAR; // CountUp, CMPA

        pwmRegs->AQCTLB.bit.ZRO = AQ_CLEAR; // Zero
        pwmRegs->AQCTLB.bit.CAU = AQ_SET;   // CountUp, CMPA

        ENABLE_PROTECTED_REGISTER_WRITE_MODE;
        // We know better than the hardware when MEP changes. :)
        pwmRegs->HRCNFG.all &= ~(PWM_HRCNFG_HRLOAD_BITS | PWM_HRCNFG_CTLMODE_BITS | PWM_HRCNFG_EDGMODE_BITS | PWM_HRCNFG_AUTOCONV_BITS);
        pwmRegs->HRCNFG.all |= PWM_HrShadowMode_CTR_EQ_0 | PWM_HrControlMode_Duty | PWM_HrEdgeMode_Falling;
        DISABLE_PROTECTED_REGISTER_WRITE_MODE;

        pwmRegs->TBCTL.all &= ~(PWM_TBCTL_CTRMODE_BITS);  // Count up and start!
    }
    EPwm1Regs.TBCTL.all  |= PWM_TBCTL_SWFSYNC_BITS; // Force synchronization of counters
    pwm_applyRatio(g_period);
}

void pwm_tick() {
    if((systick_get() & 0x0003)== 0) {
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
    uint32_t count;
    uint64_t high;
    uint16_t CMP1, CMP2;
    uint16_t mep1, mep2;
    high0 = ((uint32_t)(uint16_t)period) * ((uint32_t)(uint16_t)(MEP_ScaleFactor));
    high = (((uint64_t)high0) * ((uint64_t)g_ratio)) << 1;
    high += (1ul<<31)-1u;
    count = high>>32;
    CMP1 = (count / ((uint16_t)MEP_ScaleFactor))>>1;
    CMP2 = CMP1;

    count -= (((uint32_t)CMP1)*((uint32_t)MEP_ScaleFactor))<<1;
    mep1 = count/2;
    mep2 = count-mep1;
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
