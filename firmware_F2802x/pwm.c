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

#define EPWMA_A_REGS (EPwm3Regs)
#define EPWMA_B_REGS (EPwm4Regs)

#define PIE_VECT_INT_A (PieVectTable.EPWM3_INT)
#define PIE_VECT_INT_B (PieVectTable.EPWM4_INT)
#define EPWMA_PIE_FLAGS (PIE_InterruptSource_EPWM3 | PIE_InterruptSource_EPWM4)

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
static __interrupt void epwmA_ISR(void);
static __interrupt void epwmB_ISR(void);

void pwm_init() {
    uint32_t ch;
    int sfoStatus;
    uint16_t intr_state;

    ENABLE_PROTECTED_REGISTER_WRITE_MODE;
    GpioCtrlRegs.GPAPUD.bit.GPIO4 = GPIO_PullUp_Disable;
    GpioCtrlRegs.GPAMUX1.bit.GPIO4 = GPIO_4_Mode_EPWM3A;

    GpioCtrlRegs.GPAPUD.bit.GPIO6 = GPIO_PullUp_Disable;
    GpioCtrlRegs.GPAMUX1.bit.GPIO6 = GPIO_6_Mode_EPWM4A;
    DISABLE_PROTECTED_REGISTER_WRITE_MODE;

#ifdef DELTA_SIGMA
    // Proper ordering from the reference manual for enabling interrupts
    // 1. Disable global interrupts (CPU INTM flag)
    intr_state = __disable_interrupts();

    // 2. Disable ePWM interrupts
    EPWMA_A_REGS.ETSEL.bit.INTEN = 0;
    EPWMA_B_REGS.ETSEL.bit.INTEN = 0;

    // 3. Set TBCLKSYNC=0
    ENABLE_PROTECTED_REGISTER_WRITE_MODE;
    SysCtrlRegs.PCLKCR0.bit.TBCLKSYNC = 0;

#endif

    CLK_enablePwmClock(myClk, PWM_Number_3);
    CLK_enablePwmClock(myClk, PWM_Number_4);
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
            pwmRegs = &EPWMA_A_REGS;
        } else {
            pwmRegs = &EPWMA_B_REGS;
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
    EPWMA_A_REGS.ETCLR.bit.INT = 1;
    EPWMA_B_REGS.ETCLR.bit.INT = 1;
    //PieCtrlRegs.PIEIFR3.bit.

    // 7. Enable ePWM interrupts
    ENABLE_PROTECTED_REGISTER_WRITE_MODE;
    PIE_VECT_INT_A = &epwmA_ISR;
    PIE_VECT_INT_B = &epwmB_ISR;
    DISABLE_PROTECTED_REGISTER_WRITE_MODE;

    PieCtrlRegs.PIEIER3.all |= EPWMA_PIE_FLAGS;

    EPWMA_A_REGS.ETSEL.bit.INTEN = 1;
    EPWMA_B_REGS.ETSEL.bit.INTEN = 1;

    // 8. Enable global interrupts
    __restore_interrupts(intr_state);
    IER |= M_INT3; // Enable group 3 interrupt (must be after restoring interrupts since that would just clear it)
#endif

    pwm_applyRatio(g_period);

    ENABLE_PROTECTED_REGISTER_WRITE_MODE;
    SysCtrlRegs.PCLKCR0.bit.TBCLKSYNC = 1;       // Start all the timers synced
    DISABLE_PROTECTED_REGISTER_WRITE_MODE;

    EPWMA_A_REGS.TBCTL.all  |= PWM_TBCTL_SWFSYNC_BITS; // Force synchronization of counters
}

void pwm_tick() {
    if((systick_get() & 0x0003)== 0) {
        int sfoStatus = SFO();
        switch(sfoStatus) {
        case SFO_COMPLETE:
            pwm_applyRatio(g_period); // apply new MEP value
            break;
        case SFO_ERROR:
            error();
            break;
        }
    }
}
void pwm_setRatio(uint32_t ratio) {

    g_ratio = ratio;
    pwm_applyRatio(g_period);
}

void pwm_setPeriod(uint16_t period) {
    if(period < 16)
        period = 16; // minimum for PWM generator

    // CCR1 may not immediately load, so try to load it manually?
    // Recalculate and reapply ratio
    if(period < g_period) { // reducing period
        pwm_applyRatio(period);
        EPWMA_A_REGS.TBPRD = g_period-1;
        EPWMA_B_REGS.TBPRD = g_period-1;
    } else {
        EPWMA_A_REGS.TBPRD = g_period-1;
        EPWMA_B_REGS.TBPRD = g_period-1;
        pwm_applyRatio(period);
    }
    // And resynchronize the second channel.
    EPWMA_B_REGS.TBPHS.half.TBPHS = g_period>>1;       // Phase offset of 180 degrees
    EPWMA_A_REGS.TBCTL.all  |= PWM_TBCTL_SWFSYNC_BITS;    // Force synchronization of counters
    g_period = period;
}
// Manual says MEP is number per clock cycle;
#define MAX_32_D ((double)4294967295.0)

static void pwm_applyRatio(uint16_t period) {
    uint16_t intr_state;
    uint16_t meps, meps1, meps2;
    uint16_t high =   (((uint64_t)g_ratio)*((uint64_t)period))>>32;
    uint64_t meps_HR = ((uint64_t)g_ratio)*((uint64_t)period)*((uint64_t)MEP_ScaleFactor_16);
    meps_HR =  meps_HR - ((((uint64_t)high)*((uint64_t)MEP_ScaleFactor_16))<<32);
    meps = meps_HR >> 32; // MEPS count is the high 8 bits;
    //meps = meps + 0x0040; // Rounding

    // Disable so we don't get weird interactions with the interrupt handler
    intr_state = __disable_interrupts();
    mepsBase = meps >> 8;
    mepsFrac = meps & 0xFF;
    meps1 = meps>>8;
    meps2 = meps>>8;
    EPWMA_A_REGS.CMPA.all = (((uint32_t)high)<<16) + (meps1 << 8);
    EPWMA_B_REGS.CMPA.all = (((uint32_t)high)<<16) + (meps2 << 8);
    __restore_interrupts(intr_state);
}

void pwm_applyWhole(uint16_t x) {
    EPWMA_A_REGS.CMPA.half.CMPA = x;
    EPWMA_B_REGS.CMPA.half.CMPA = x;
}
void pwm_applyMEP(uint16_t x) {
    uint16_t a = x>>1;
    EPWMA_A_REGS.CMPA.half.CMPAHR = a<<8;
    EPWMA_B_REGS.CMPA.half.CMPAHR = (x-a)<<8;
}
//
// sciaRxFifoIsr -
//
#ifdef DELTA_SIGMA
// See http://www.ti.com/lit/an/slyt076/slyt076.pdf for details
// DAC_IN is mepsFrac, a 16-bit value

#define DS_N (16)

static uint16_t dacout2;

static __interrupt void epwmA_ISR(void) {
    static uint16_t toggle = 0;
    uint16_t x;
    uint32_t sigma_out, delta_out;
    // Calculate first CMPA
    static uint32_t pwmA_fraction_sigma = (1ul<<(DS_N+2-1));    // only touched by ISR, so no need for volatile.
    uint32_t delta = 0ul;
    if (pwmA_fraction_sigma & (1ul << (DS_N+2-1 ))) // if bit 16+2
        delta = (3ul << DS_N);
    delta_out = ((uint32_t)mepsFrac) + delta;
    sigma_out = delta_out + pwmA_fraction_sigma;
    pwmA_fraction_sigma = sigma_out;
    x = 0;
    if(pwmA_fraction_sigma & (1ul << (DS_N+2-1 ))) // if highest bit set?
        x=1;
    if(toggle)
        EPWMA_A_REGS.CMPA.half.CMPAHR = (mepsBase + x)<<8;
    else
        dacout2 = mepsBase + x;

    // calculate second CMPA
    delta = 0ul;
    if (x) // if bit 16+2
        delta = (3ul << DS_N);
    delta_out = ((uint32_t)mepsFrac) + delta;
    sigma_out = delta_out + pwmA_fraction_sigma;
    sigma_out = delta_out + pwmA_fraction_sigma;
    x = 0;
    if(pwmA_fraction_sigma & (1ul << (DS_N+2-1 ))) // if highest bit set?
        x=1;

    if(toggle)
        dacout2 = mepsBase + x;
    else
        EPWMA_A_REGS.CMPA.half.CMPAHR = (mepsBase + x)<<8;

    toggle = !toggle;

    // Ack the interrupt
    EPWMA_A_REGS.ETCLR.bit.INT = 1;
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP_EPWM;
    return;
}

static __interrupt void epwmB_ISR(void) {
    EPWMA_B_REGS.CMPA.half.CMPAHR = dacout2 << 8;

    EPWMA_B_REGS.ETCLR.bit.INT = 1;
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP_EPWM;
    return;
}
#endif

