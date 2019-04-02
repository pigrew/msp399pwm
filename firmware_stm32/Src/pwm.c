/*
 * pwm.c
 *
 *  Created on: Apr 1, 2019
 *      Author: nconrad
 */



#include "pwm.h"
#include "main.h"


#define DUAL_PWM

#define HRTIM (HRTIM1)
#define TMR_CH (1)
// PWMA is TD0.0
static uint16_t g_period; // period as set over UART
static uint32_t g_ratio; // ratio as set over UART

static volatile uint16_t pwmA_base;     // touched by ISR and user code,
static volatile uint32_t pwmA_fraction; // touched by ISR and user code, must disable interrupts before changing.

static void pwm_applyRatio(uint16_t period);

// Call this, assuming the device is somewhat initialized by HAL.
// This uses ONLY the timer_A unit (and not the master)
void pwm_init() {
    g_period = 0xf000;    // 10 kHz
    g_ratio = 3053247871ul;

    // Set TA's preload-enable, update register at roll-over, continuous-up mode
    HRTIM->sTimerxRegs[TMR_CH].TIMxCR |=  HRTIM_TIMCR_PREEN | HRTIM_TIMCR_TRSTU | HRTIM_TIMCR_CONT; // Setup timer A
    HRTIM->sTimerxRegs[TMR_CH].REPxR &= ~0x000F; // set preload to zero.

    HRTIM->sTimerxRegs[TMR_CH].PERxR = g_period;

    HRTIM->sTimerxRegs[TMR_CH].SETx1R = HRTIM_SET1R_PER; // set 1 on period
    HRTIM->sTimerxRegs[TMR_CH].RSTx1R = HRTIM_RST1R_CMP1; // reset 1 on cmp1

    HRTIM->sTimerxRegs[TMR_CH].SETx2R = HRTIM_SET2R_CMP2; // set 1 on period
    HRTIM->sTimerxRegs[TMR_CH].RSTx2R = HRTIM_RST2R_CMP3; // reset 1 on cmp1

    pwm_setPeriod(g_period); // will set both period and ratio

    // And enable the outputs+timers
#if (TMR_CH == 0)
    HRTIM->sCommonRegs.OENR |= HRTIM_OENR_TA1OEN | HRTIM_OENR_TA2OEN;
    HRTIM->sMasterRegs.MCR |= HRTIM_MCR_TACEN;
#elif (TMR_CH == 1)
    HRTIM->sCommonRegs.OENR |= HRTIM_OENR_TB1OEN | HRTIM_OENR_TB2OEN;
    HRTIM->sMasterRegs.MCR |= HRTIM_MCR_TBCEN;
#endif
    // Enable the interrupt
    HRTIM->sTimerxRegs[TMR_CH].TIMxDIER |= HRTIM_TIMDIER_RSTIE;

}

void pwm_setPeriod(uint16_t period) {
    if(period < 16)
        period = 16; // minimum for PWM generator
    period = period & ~(0x000f); // last few bits are ignored by timer

    // CCR1 may not immediately load, so try to load it manually?
    // Recalculate and reapply ratio
    if(period < g_period) { // reducing period
        pwm_applyRatio(period);
        HRTIM->sTimerxRegs[TMR_CH].PERxR = period; // must subtract 1 to get proper period!
    } else {
    	HRTIM->sTimerxRegs[TMR_CH].PERxR = period; // must subtract 1 to get proper period!
        pwm_applyRatio(period);
    }
    g_period = period;
}
#define MAX_32_D ((double)4294967295.0)


static void pwm_applyRatio(uint16_t period) {
    // float is only 24-bit, so we need double-precision if using floating-point for math.
    double r = ((double)g_ratio)/MAX_32_D;
    double high = r*period;
    pwmA_base = (uint16_t)high;
    uint32_t actualBase = (((double)pwmA_base)/ ((double)period))*MAX_32_D;

    r = ((double)(g_ratio - actualBase)) * ((double)period); // remainder

    pwmA_fraction = ((uint32_t)(r))>>2;  // shift right two bits to make room for accumulators in delta-sigma method

    HRTIM->sTimerxRegs[TMR_CH].CMP1xR = pwmA_base;
#ifdef DUAL_PWM
    HRTIM->sTimerxRegs[TMR_CH].CMP2xR = period/2;
    HRTIM->sTimerxRegs[TMR_CH].CMP3xR = ((uint32_t)period/2+pwmA_base) % period;
#endif

}

void pwm_setRatio(uint32_t ratio) {

    g_ratio = ratio;
    pwm_applyRatio(g_period);
}

#define HIGH_BYTE(x) (*(((uint8_t*)&x)+3))
#define HIGH_WORD(x) (*(((uint16_t*)&x)+1))

// delta-sigma modulator, see http://www.ti.com/lit/an/slyt076/slyt076.pdf
#define DS_N (30)
//RAMFUNC
void HAL_HRTIM_CounterResetCallback(HRTIM_HandleTypeDef * hhrtim,
                                              uint32_t TimerIdx)
{

	  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_RESET);
	/*static int xt = 0;
	xt = !xt;
	if(xt) {
	    HRTIM->sTimerxRegs[TMR_CH].CMP1xR = pwmA_base;
	} else {

	    HRTIM->sTimerxRegs[TMR_CH].CMP1xR = 0x40;
	}
	return;*/
    static uint32_t pwmA_fraction_sigma = (1ul<<31);    // only touched by ISR, so no need for volatile.
#ifdef DUAL_PWM
    static uint8_t toggle = 0;
#endif
    //PAOUT_H |= (1 << (4)); // set p2.4

    uint32_t delta = 0;
    if (HIGH_WORD(pwmA_fraction_sigma) & (0x8000)) // if highest bit set?
        delta = (3ul << DS_N);
    uint32_t delta_out = pwmA_fraction + delta;
    uint32_t sigma_out = delta_out + pwmA_fraction_sigma;
    pwmA_fraction_sigma = sigma_out;
    uint16_t x = 0;
    if(HIGH_WORD(pwmA_fraction_sigma) & (0x8000)) // if highest bit set?
        x=1;
#ifdef DUAL_PWM
    if(toggle)
    	HRTIM->sTimerxRegs[TMR_CH].CMP1xR = x + pwmA_base;
    else
    	HRTIM->sTimerxRegs[TMR_CH].CMP3xR = ((uint32_t)g_period/2+pwmA_base+x) % g_period;

    delta = 0;
    if (HIGH_WORD(pwmA_fraction_sigma) & (0x8000)) // if highest bit set?
        delta = (3ul << DS_N);
    delta_out = pwmA_fraction + delta;
    sigma_out = delta_out + pwmA_fraction_sigma;
    pwmA_fraction_sigma = sigma_out;
    x = 0;
    if(HIGH_WORD(pwmA_fraction_sigma) & (0x8000)) // if highest bit set?
        x=1;
    if(toggle)
    	HRTIM->sTimerxRegs[TMR_CH].CMP3xR = ((uint32_t)g_period/2+pwmA_base+x) % g_period;
    else
    	HRTIM->sTimerxRegs[TMR_CH].CMP1xR = x + pwmA_base;
    toggle = !toggle;
#else
    HRTIM->sTimerxRegs[TMR_CH].CMP1xR = x + pwmA_base;
    HRTIM->sTimerxRegs[TMR_CH].CMP3xR = ((uint32_t)g_period/2+pwmA_base+x) % period;

#endif

    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_SET);

   // PAOUT_H &= ~(1 << (4)); // clear p2.4
}

