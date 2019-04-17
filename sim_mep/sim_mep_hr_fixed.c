#include <stdio.h> 
#include <stdint.h>
#include <math.h>
#include <stdlib.h>

uint16_t MEP_ScaleFactor_16 = 0x88CD;

static uint32_t g_ratio = 0xB5FCDD7F; // 32-bit
static uint16_t g_period = 0xFFF0;    // 16-bit
static void pwm_applyRatio(uint16_t period);

uint32_t pwmreg1;
uint32_t pwmreg2;

#define MAX_32_D ((double)4294967296.0)


int main() {
	for(int i=0; i<1000; i++) {
		/*g_ratio = 
			 ((uint32_t)rand() & 0xFFFF)<<16
			| (rand() & 0xFFFF) ;*/
		g_ratio = 0xB0000000 |
			((uint32_t)rand() & 0x0FFF)<<16
			| (rand() & 0xFFFF) ;
		g_ratio &= ~0xFFF00000;
		g_ratio |=  0x71200000;
		pwm_applyRatio(g_period);
		
		uint16_t CMPA = pwmreg1 >> 16;
		uint16_t CMPB = pwmreg2 >> 16;
		uint16_t mepA = (pwmreg1>>8)&0x00FF;
		uint16_t mepB = (pwmreg2>>8)&0x00FF;
		uint16_t period = g_period;
		
		double r = ((double)g_ratio)/MAX_32_D;
		double mepsf = ((double)MEP_ScaleFactor_16)/256.0;
		
		double final = ((CMPA+mepA/mepsf)/(double)period +
						(CMPB+mepB/mepsf)/(double)period)/2.0;
		double rminus = ((CMPA+mepA/mepsf)/(double)period +
						(CMPB+(mepB-1)/mepsf)/(double)period)/2.0;
		double rplus =  ((CMPA+mepA/mepsf)/(double)period +
						(CMPB+(mepB+1)/mepsf)/(double)period)/2.0;
		double roffset = final-r;
		double rplusoffset = rplus-r;
		double rminusoffset = rminus-r;
		
		double rtol = 1.0/(1<<30);
		//printf("reg=%x\n",reg);
		if(((fabs(rplusoffset)+rtol) < fabs(roffset)) || ((fabs(rminusoffset)+rtol) < fabs(roffset))/*|| (mepA >= mepsf)*/) {
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
			printf("count=%x\n", (mepA + mepB) + (MEP_ScaleFactor_16>>8)*(CMPA+CMPB));
			printf("------------------\n");
			printf("\n");
		}
	
	
	}
	return 0;
}
uint16_t mepsBase;
uint16_t mepsFrac;
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
	pwmreg1 = (((uint32_t)high)<<16) + (meps1 << 8);
	pwmreg2 = (((uint32_t)high)<<16) + (meps2 << 8);

}
