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
#include "main.h"

#include "common/include/clk.h"
#include "common/include/gpio.h"
#include "common/include/pwm.h"

PWM_Handle myPwm1, myPwm2;

static uint16_t g_period   = 0x0050;
static uint32_t g_duration = 0x00388000;
static uint32_t g_ratio = 3053247871ul;

void pwm_init(CLK_Handle myClk, GPIO_Handle myGpio) {
    uint32_t ch;

    myPwm1 = PWM_init((void *)PWM_ePWM1_BASE_ADDR, sizeof(PWM_Obj));
    myPwm2 = PWM_init((void *)PWM_ePWM2_BASE_ADDR, sizeof(PWM_Obj));

    GPIO_setPullUp(myGpio, GPIO_Number_0, GPIO_PullUp_Disable);
    GPIO_setMode(myGpio, GPIO_Number_0, GPIO_0_Mode_EPWM1A);

    GPIO_setPullUp(myGpio, GPIO_Number_2, GPIO_PullUp_Disable);
    GPIO_setMode(myGpio, GPIO_Number_2, GPIO_2_Mode_EPWM2A);


    CLK_enableTbClockSync(myClk);
    CLK_enablePwmClock(myClk, PWM_Number_1);
    CLK_enablePwmClock(myClk, PWM_Number_2);
    CLK_enableHrPwmClock(myClk);

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
