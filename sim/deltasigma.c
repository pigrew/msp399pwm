#include <stdio.h>
#include <stdint.h>


void __enable_interrupt() {
}
void __disable_interrupt() {
}

#define __interrupt /* */

#define MAX_32_D ((double)4294967295.0)




static uint16_t g_period; // period as set over UART
static uint32_t g_ratio; // ratio as set over UART

static uint16_t pwmA_base;     // touched by ISR and user code,
static uint32_t pwmA_fraction; // touched by ISR and user code, must disable interrupts before changing.

static uint16_t TD0CCR1;
static uint16_t PAOUT_H;

static void pwm_applyRatio(uint16_t period) {
    // float is only 24-bit, so we need double-precision if using floating-point for math.
    double r = ((double)g_ratio)/MAX_32_D;
    double high = r*period;
    pwmA_base = (uint16_t)high;
    uint32_t actualBase = (((double)pwmA_base)/ ((double)period))*MAX_32_D;

    r = ((double)(g_ratio - actualBase)) * ((double)period); // remainder

    __disable_interrupt();
    pwmA_fraction = ((uint32_t)(r))>>2;  // shift right two bits to make room for accumulators in delta-sigma method
    __enable_interrupt();

    TD0CCR1 = pwmA_base;
}

#define HIGH_BYTE(x) (*(((uint8_t*)&x)+3))
#define HIGH_WORD(x) (*(((uint16_t*)&x)+1))
// delta-sigma modulator, see http://www.ti.com/lit/an/slyt076/slyt076.pdf
#define DS_N (30)
#pragma vector=TIMER0_D0_VECTOR
__attribute__((ramfunc))
__interrupt
void TIMER0_D0_ISR(void) { // 3.13us
    static uint32_t pwmA_fraction_sigma;    // only touched by ISR, so no need for volatile.

    PAOUT_H |= (1 << (6)); // set p2.6
    uint32_t delta = 0;
    if (HIGH_WORD(pwmA_fraction_sigma) & (0x8000)) // if highest bit set?
        delta = (3ul << DS_N);
    uint32_t delta_out = pwmA_fraction + delta;
    uint32_t sigma_out = delta_out + pwmA_fraction_sigma;
    pwmA_fraction_sigma = sigma_out;
    uint16_t x = 0;
    if(HIGH_WORD(pwmA_fraction_sigma) & (0x8000)) // if highest bit set?
        x=1;
    TD0CCR1 = x + pwmA_base;

    PAOUT_H &= ~(1 << (6)); // clear p2.6
}


int main() {
	g_period = 0x9600;
	uint32_t ratio_in = 3052222982;
	for(g_period = 0x9600 ; g_period <  0x9601; g_period++) {
		//ratio_in = 1/((double)g_period)*MAX_32_D;
		for(int32_t r = -20; r<=20; r++) {
			g_ratio = ratio_in + r;
			pwm_applyRatio(g_period);
			unsigned int n=0, d=0;
			for(int i=0; i<10000000; i++) {
				TIMER0_D0_ISR();
				d++;
				if(TD0CCR1 != pwmA_base)
					n++;
			}
			printf("%3d, %4d , %8x, %0.8f\n", r, pwmA_base, pwmA_fraction, ((double)n)/d);
		}
	}
	return 0;
}
