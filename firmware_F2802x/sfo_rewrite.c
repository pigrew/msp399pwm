
#include "F2802x_Device.h"     // F2802x Headerfile Include File
#include "cpu.h"
#include "sfo_v6.h"

extern int MEP_ScaleFactor;

extern volatile struct EPWM_REGS *ePWM[PWM_CH] =
             {  &EPwm1Regs, &EPwm1Regs, &EPwm2Regs};

static uint16_t MEP_SF[PWM_CH];

// page 1e
static uint16_t HR_Result24a; // x26/x35
static uint16_t HR_Result24b; // x27/x36
static uint16_t HRCNFG_save;  // x28/x37
static int state = 0;         // x29/x38
static uint16_t CMPAHR_save;  // x2a/x39
static uint16_t ePWM_i;       // x2b/x3a which ePWM to use??? Never changes???

int SFO2() {
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
            EPwm1Regs.rsvd4[0] = 0x50;    // #22
        } else {
            uint16_t cfg = ePWM[ePWM_i]->HRCNFG.all; // get HRCNFG
            HRCNFG_save = cfg;
            CMPAHR_save = ePWM[ePWM_i]->CMPA.half.CMPAHR;
            ePWM[ePWM_i]->HRCNFG.all &= ~(0x0003); //change config, disable edge mode bits?
            ePWM[ePWM_i]->HRCNFG.all |= 0x0018; //change config, disable edge mode bits?
            uint16_t ah = EPwm1Regs.HRPWR.all & ~(0x03c0); // enable power???
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
        if(0 == EPwm1Regs.HRPWR.all & 0x0010) {
            EPwm1Regs.HRPWR.all &= ~(0x0004);
            state = 3;
            HR_Result24b = EPwm1Regs.rsvd4[2]; // #24 (approx 35064)
        }

        DISABLE_PROTECTED_REGISTER_WRITE_MODE;
	    break;
	case 3:
        ENABLE_PROTECTED_REGISTER_WRITE_MODE;
        if(ePWM_i == 0)
            EPwm1Regs.rsvd4[0] = 0xc8;  // #22
        else {
            ePWM[ePWM_i]->CMPA.half.CMPAHR = 0xc800;
        }
        EPwm1Regs.rsvd4[2] = 0;
        EPwm1Regs.rsvd4[3] = 0;
        EPwm1Regs.HRPWR.all |= 0x0004;
        DISABLE_PROTECTED_REGISTER_WRITE_MODE;

        for(i=0; i<0x32; i++) {
            asm(" nop");
            // Delay loop?????
        }
        state = 4;
	case 4:
	    ENABLE_PROTECTED_REGISTER_WRITE_MODE;
	    if(((EPwm1Regs.HRPWR.all & 0x0010)>>4) == 0) {
	        EPwm1Regs.HRPWR.all &= ~(0x0004); // clear bit 2
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
	    uint16_t sf = 240.0/c + 0.5;

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
