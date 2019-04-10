#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <stdlib.h>

int MEP_ScaleFactor;

static uint32_t g_ratio = 0xB5FCDD7F;
static uint32_t g_period = 16;

static void pwm_applyRatio(uint16_t period);

int main() {
	MEP_ScaleFactor = 151;
	for(int i=0; i<100000; i++) {
		g_ratio = (rand() & 0xFFFF)<<16 | (rand() & 0xFFFF);
		pwm_applyRatio(g_period);
	}
	return 0;
}

#define MAX_32_D ((double)4294967295.0)

static void pwm_applyRatio(uint16_t period) {
	double r = ((double)g_ratio)/MAX_32_D;
    double high = r*(period*2*MEP_ScaleFactor)+0.5; // 0.5 to fix rounding
	uint32_t count = (uint32_t)high;
	//printf("high=%f\n", high);
	//printf("count=%d\n", count);
	uint32_t CMPA = high / (2*MEP_ScaleFactor);
	uint32_t CMPB = CMPA;
	
	//printf("CMPA=%d, CMPb=%d\n", CMPA, CMPB);
	count -= CMPA*(2*MEP_ScaleFactor);
	uint32_t mepA = count/2;
	uint32_t mepB = count-mepA;
	if(mepB == MEP_ScaleFactor) {
		CMPB = CMPA + 1;
		mepB = 0;
	}
	
	double final = ((CMPA+mepA/(double)MEP_ScaleFactor)/(double)period +
	                (CMPB+mepB/(double)MEP_ScaleFactor)/(double)period)/2.0;
	//printf(" r=%f\n", final);
	//printf("r0=%f\n", r);
	double rminus = ((CMPA+mepA/(double)MEP_ScaleFactor)/(double)period +
	                (CMPB+(mepB-1)/(double)MEP_ScaleFactor)/(double)period)/2.0;
	double rplus = ((CMPA+mepA/(double)MEP_ScaleFactor)/(double)period +
	                (CMPB+(mepB+1)/(double)MEP_ScaleFactor)/(double)period)/2.0;
	double roffset = fabs(final-r);
	double rplusoffset = fabs(rplus-r);
	double rminusoffset = fabs(rminus-r);
	
	if((rplusoffset < roffset) || (rminusoffset < roffset) || mepA >= MEP_ScaleFactor) {
		printf("FAIL %x\n", g_ratio);
		printf(" ro=%f\n", roffset);
		printf("r-o=%f\n", rminusoffset);
		printf("r+o=%f\n", rplusoffset);
	}
}