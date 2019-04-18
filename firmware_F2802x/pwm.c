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

#define DELTA_SIGMA

#define PIEACK_GROUP_EPWM (PIEACK_GROUP3)

uint16_t MEP_ScaleFactor=0;
uint16_t MEP_ScaleFactor_16;
#pragma DATA_ALIGN(MEP_ScaleFactor_16,2); // Align for MAC usage


static uint16_t g_period   = 0xFFFF;
//static uint32_t g_duration = 0xB5FCDD00;
static uint32_t g_ratio = 0xB5FCDD7F;
#pragma DATA_ALIGN(g_period,2); // Align for MAC usage
#pragma DATA_ALIGN(g_ratio,2); // Align for MAC usage


static uint16_t mepsBase;
// mepsFrac is 8-bit (0x00 to 0xFF)
static uint16_t mepsFrac;

static void pwm_applyRatio(uint16_t period);
__interrupt void epwm1_ISR(void);
__interrupt void epwm2_ISR(void);

void pwm_init() {
    uint32_t ch;
    int sfoStatus;

    ENABLE_PROTECTED_REGISTER_WRITE_MODE;
    GpioCtrlRegs.GPAPUD.bit.GPIO0 = GPIO_PullUp_Disable;
    GpioCtrlRegs.GPAPUD.bit.GPIO2 = GPIO_PullUp_Disable;
    GpioCtrlRegs.GPAMUX1.bit.GPIO0 = GPIO_0_Mode_EPWM1A;
    GpioCtrlRegs.GPAMUX1.bit.GPIO2 = GPIO_2_Mode_EPWM2A;
    DISABLE_PROTECTED_REGISTER_WRITE_MODE;

#ifdef DELTA_SIGMA
    // Proper ordering from the reference manual for enabling interrupts
    // 1. Disable global interrupts (CPU INTM flag)
    DISABLE_INTERRUPTS;

    // 2. Disable ePWM interrupts
    EPwm1Regs.ETSEL.bit.INTEN = 0;
    EPwm2Regs.ETSEL.bit.INTEN = 0;

    // 3. Set TBCLKSYNC=0
    ENABLE_PROTECTED_REGISTER_WRITE_MODE;
    SysCtrlRegs.PCLKCR0.bit.TBCLKSYNC = 0;

#endif

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
        pwmRegs->TBPRD = g_period-1;

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
#ifdef DELTA_SIGMA
        pwmRegs->ETPS.bit.INTPRD = ET_1ST;
        pwmRegs->ETSEL.bit.INTSEL = ET_CTR_ZERO; // Interrupt on zero-count
        pwmRegs->ETSEL.bit.SOCAEN = 0; // Disable ADC triggers
        pwmRegs->ETSEL.bit.SOCBEN = 0;
#endif
    }

#ifdef DELTA_SIGMA
    // 5. Set TBCLKSYNC=1
    // 6. Clear any spurious ePWM flags (including PIEIFR)
    EPwm1Regs.ETCLR.bit.INT = 1;
    EPwm2Regs.ETCLR.bit.INT = 1;
    //PieCtrlRegs.PIEIFR3.bit.

    // 7. Enable ePWM interrupts
    ENABLE_PROTECTED_REGISTER_WRITE_MODE;
    PieVectTable.EPWM1_INT = &epwm1_ISR;
    PieVectTable.EPWM2_INT = &epwm2_ISR;
    DISABLE_PROTECTED_REGISTER_WRITE_MODE;

    PieCtrlRegs.PIEIER3.all |= PIE_InterruptSource_EPWM1 | PIE_InterruptSource_EPWM2;
    IER |= M_INT3; // Enable group 3 interrupt

    EPwm1Regs.ETSEL.bit.INTEN = 1;
    EPwm2Regs.ETSEL.bit.INTEN = 1;

    // 8. Enable global interrupts
    ENABLE_INTERRUPTS;
#endif

    pwm_applyRatio(g_period);

    EALLOW;
    SysCtrlRegs.PCLKCR0.bit.TBCLKSYNC = 1;       // Start all the timers synced
    EDIS;

    EPwm1Regs.TBCTL.all  |= PWM_TBCTL_SWFSYNC_BITS; // Force synchronization of counters
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
    uint16_t high =   (((uint64_t)g_ratio)*((uint64_t)period))>>32;
    uint64_t meps_HR = ((uint64_t)g_ratio)*((uint64_t)period)*((uint64_t)MEP_ScaleFactor_16);
    meps_HR =  meps_HR - ((((uint64_t)high)*((uint64_t)MEP_ScaleFactor_16))<<32);
    uint16_t meps = meps_HR >> 32; // MEPS count is the high 8 bits;
    //meps = meps + 0x0040; // Rounding
    mepsBase = meps >> 8;
    mepsFrac = meps & 0xFF;
    uint16_t meps1 = meps>>8;
    uint16_t meps2 = meps>>8;
    EPwm1Regs.CMPA.all = (((uint32_t)high)<<16) + (meps1 << 8);
    EPwm2Regs.CMPA.all = (((uint32_t)high)<<16) + (meps2 << 8);

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


//
// sciaRxFifoIsr -
//
#ifdef DELTA_SIGMA
__interrupt void epwm1_ISR(void) {
    //
    // Clear INT flag for this timer
    //
    EPwm1Regs.ETCLR.bit.INT = 1;

    //
    // Acknowledge this interrupt to receive more interrupts from group 3
    //
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP_EPWM;
    return;
}
__interrupt void epwm2_ISR(void) {
    //
    // Clear INT flag for this timer
    //
    EPwm2Regs.ETCLR.bit.INT = 1;

    //
    // Acknowledge this interrupt to receive more interrupts from group 3
    //
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP_EPWM;
    return;
}
#endif

