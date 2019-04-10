#include <stdio.h> 
#include <stdint.h>
#include <math.h>
#include <stdlib.h>

int MEP_ScaleFactor;

static uint32_t g_ratio = 0xB5FCDD7F;
static uint16_t g_period = 0xFFF0;

static void pwm_applyRatio(uint16_t period);

uint32_t pwmreg1;
uint32_t pwmreg2;
#define MAX_32_D ((double)4294967295.0)


int main() {
	MEP_ScaleFactor = 151;
	for(int i=0; i<1000; i++) {
		/*g_ratio = 
			 ((uint32_t)rand() & 0xFFFF)<<16
			| (rand() & 0xFFFF) ;*/
		g_ratio = 0xB0000000 |
			((uint32_t)rand() & 0x0FFF)<<16
			| (rand() & 0xFFFF) ;
		pwm_applyRatio(g_period);
		
		uint16_t CMPA = pwmreg1 >> 16;
		uint16_t CMPB = pwmreg2 >> 16;
		uint16_t mepA = (pwmreg1>>8)&0x00FF;
		uint16_t mepB = (pwmreg2>>8)&0x00FF;
		uint16_t period = g_period;
		
		double r = ((double)g_ratio)/MAX_32_D;
		double final = ((CMPA+mepA/(double)MEP_ScaleFactor)/(double)period +
						(CMPB+mepB/(double)MEP_ScaleFactor)/(double)period)/2.0;
		double rminus = ((CMPA+mepA/(double)MEP_ScaleFactor)/(double)period +
						(CMPB+(mepB-1)/(double)MEP_ScaleFactor)/(double)period)/2.0;
		double rplus = ((CMPA+mepA/(double)MEP_ScaleFactor)/(double)period +
						(CMPB+(mepB+1)/(double)MEP_ScaleFactor)/(double)period)/2.0;
		double roffset = fabs(final-r);
		double rplusoffset = fabs(rplus-r);
		double rminusoffset = fabs(rminus-r);
		
		double rtol = 1.0/(1<<30);
		//printf("reg=%x\n",reg);
		if(((rplusoffset+rtol) < roffset) || ((rminusoffset+rtol) < roffset) || (mepA >= MEP_ScaleFactor)) {
			printf("FAIL %x\n", g_ratio);
			printf("  r=%.12f\n", r);
			printf(" rc=%.12f\n", final);
			printf("\n");
			printf(" ro=%.12f\n", roffset);
			printf("r-o=%.12f\n", rminusoffset);
			printf("r+o=%.12f\n", rplusoffset);
			printf("\n");
			printf("A=%d,B=%d,MEPA=%d,MEPB=%d\n",CMPA, CMPB, mepA, mepB);
			//printf("count0=%x\n", (uint32_t)(high>>32));
			printf("count=%x\n", (mepA + mepB) + MEP_ScaleFactor*(CMPA+CMPB));
			printf("------------------\n");
			printf("\n");
		}
	
	
	}
	return 0;
}

static void pwm_applyRatio(uint16_t period) {
    uint32_t high0;
    high0 = ((uint32_t)(uint16_t)period) * ((uint32_t)(uint16_t)(MEP_ScaleFactor));
    uint64_t high = (((uint64_t)high0) * ((uint64_t)g_ratio)) << 1; // high = period * MEP_ScaleFactor * g_ratio * 2
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
    pwmreg1/*EPwm1Regs.CMPA.all*/ = (((uint32_t)CMP1)<<16) | (mep1<<8);
    pwmreg2/*EPwm2Regs.CMPA.all*/ = (((uint32_t)CMP2)<<16) | (mep2<<8);

}
