
#include "F2802x_Device.h"     // F2802x Headerfile Include File
#include "cpu.h"
#include "sfo_v6.h"

extern int MEP_ScaleFactor;

extern volatile struct EPWM_REGS *ePWM[PWM_CH];

static uint16_t MEP_SF[PWM_CH];

// page 1e
static uint16_t HR_Result24a; // x26/x35
static uint16_t HR_Result24b; // x27/x36
static uint16_t HRCNFG_save;  // x28/x37
static int state = 0;         // x29/x38
static uint16_t CMPAHR_save;  // x2a/x39
static uint16_t ePWM_i;       // x2b/x3a which ePWM to use??? Never changes???

#define LEN_A (0x50)
#define LEN_B (0xc8)
// A measurement is performed for these two counts. The slope delta(1/Y)/delta(LEN) seems to be proportional to the MEP count.

#define HRCNFG_HRLOAD_TEST  (0x0018)
#define HRCNFG_EDGMODE_MASK (0x0003)

// The MEP cal channel to use is encoded here.
// If CH is all 1, then the extra test channel is used.
#define HRPWR_CAL_PWR       (0x0200)
#define HRPWR_CAL_CH        (0x01c0)
#define HRPWR_MEPOFF_MASK   (0x03c0)

#define HRPWR_DONE          (0x0010)
#define HRPWR_MEASURE       (0x0004)


int SFO() {
    float a,b,c;
    int retVal = 0;
    uint16_t i;
	switch(state) {
	case 0:
	    ENABLE_PROTECTED_REGISTER_WRITE_MODE;
		EPwm1Regs.HRPWR.all = 0;
		EPwm1Regs.HRPWR.all |= 0x0008; // unknown bits...
		EPwm1Regs.HRPWR.all |= 0x0020;
		DISABLE_PROTECTED_REGISTER_WRITE_MODE;
		state = 1;
		HR_Result24a = 0;
		HR_Result24b = 0;
		ePWM_i = 0;
		break;
	case 1:
        ENABLE_PROTECTED_REGISTER_WRITE_MODE;
        if(ePWM_i == 0) {
            EPwm1Regs.HRPWR.all &= ~(0x03c0); // #21 Enable power?
            EPwm1Regs.rsvd4[0] = LEN_A;    // #22
        } else {
            HRCNFG_save = ePWM[ePWM_i]->HRCNFG.all;
            CMPAHR_save = ePWM[ePWM_i]->CMPA.half.CMPAHR;
            ePWM[ePWM_i]->HRCNFG.all &= ~(HRCNFG_EDGMODE_MASK);          // disable edge mode bits?
            ePWM[ePWM_i]->HRCNFG.all |= HRCNFG_HRLOAD_TEST;         // Enable test mode?
            uint16_t ah = EPwm1Regs.HRPWR.all & ~(HRPWR_MEPOFF_MASK);    // enable power
            EPwm1Regs.HRPWR.all = (((ePWM_i-1)|0x0008) & 0x000f)<<6 | ah;
            ePWM[ePWM_i]->CMPA.half.CMPAHR = 0x5000;
        }
        EPwm1Regs.rsvd4[2] = 0; // #24
        EPwm1Regs.rsvd4[3] = 0; // #25
        EPwm1Regs.HRPWR.all |= 0x0004; // #21, unknown bit, starts a measurement, it seems.
        DISABLE_PROTECTED_REGISTER_WRITE_MODE;
        state = 2;
        break;
	case 2:
        ENABLE_PROTECTED_REGISTER_WRITE_MODE;
        if(0 == (EPwm1Regs.HRPWR.all & 0x0010)) {
            EPwm1Regs.HRPWR.all &= ~(HRPWR_MEASURE);
            state = 3;
            HR_Result24b = EPwm1Regs.rsvd4[2]; // #24 (approx 35064)
        }

        DISABLE_PROTECTED_REGISTER_WRITE_MODE;
	    break;
	case 3:
        ENABLE_PROTECTED_REGISTER_WRITE_MODE;
        if(ePWM_i == 0)
            EPwm1Regs.rsvd4[0] = LEN_B;  // #22
        else {
            ePWM[ePWM_i]->CMPA.half.CMPAHR = 0xc800;
        }
        EPwm1Regs.rsvd4[2] = 0;
        EPwm1Regs.rsvd4[3] = 0;
        EPwm1Regs.HRPWR.all |= HRPWR_MEASURE;
        DISABLE_PROTECTED_REGISTER_WRITE_MODE;

        for(i=0; i<0x32; i++) {
            asm(" nop");
            // Delay loop?????
        }
        state = 4;
	case 4:
	    ENABLE_PROTECTED_REGISTER_WRITE_MODE;
	    if(((EPwm1Regs.HRPWR.all & HRPWR_DONE)>>4) == 0) {
	        EPwm1Regs.HRPWR.all &= ~(HRPWR_MEASURE); // clear bit 2
	        HR_Result24a = EPwm1Regs.rsvd4[2]; // #24
	        state = 5;
	    }
	    DISABLE_PROTECTED_REGISTER_WRITE_MODE;


	    break;
	case 5:
	    ENABLE_PROTECTED_REGISTER_WRITE_MODE;
	    a = 1.0f / (float)HR_Result24a;
	    b = 1.0f / (float)HR_Result24b;
	    c = a-b;
	    c = c*65535.0f;
	    uint16_t sf = (2*(LEN_B-LEN_A))/c + 0.5;

	    MEP_SF[ePWM_i] = sf; // *+XAR4[0], AR6
        MEP_ScaleFactor = sf;
        if(MEP_SF[0] > 0xFF) {
	        retVal = 2;
	    } else {
	        EPwm1Regs.HRMSTEP = MEP_SF[0];
	        retVal = 1;
	    }
        state = 1;
	    if(ePWM_i != 0) {
            ePWM[ePWM_i]->HRCNFG.all = HRCNFG_save;
            ePWM[ePWM_i]->CMPA.half.CMPAHR = CMPAHR_save;
	    }
	    DISABLE_PROTECTED_REGISTER_WRITE_MODE;
	    break;
	}
	return retVal;
}
