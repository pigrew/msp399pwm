
#include "F2802x_Device.h"     // F2802x Headerfile Include File

extern int MEP_ScaleFactor;

extern volatile struct EPWM_REGS *ePWM[PWM_CH] =
             {  &EPwm1Regs, &EPwm1Regs, &EPwm2Regs};

// page 1e
static int state = 0;
static int x20;
static int x21;
static int x25;

int SFO2() {
    int retVal = 0;
	switch(state) {
	case 0:
	    ENABLE_PROTECTED_REGISTER_WRITE_MODE;
		EPwm1Regs.HRPWR.all = 0;
		EPwm1Regs.HRPWR.all |= 0x0008; // unknown bits...
		EPwm1Regs.HRPWR.all |= 0x0020;
		DISABLE_PROTECTED_REGISTER_WRITE_MODE;
		state = 1;
		x20 = 0;
		x21 = 0;
		x25 = 0;
		break;
	case 1:
        ENABLE_PROTECTED_REGISTER_WRITE_MODE;
        if(x25 == 0) {
            EPwm1Regs.HRPWR.all &= ~(0x03c0); // #21 Enable power?
            EPwm1Regs.rsvd4[0] = 0x50;    // #22
        } else {
             // ???
        }
        EPwm1Regs.rsvd4[2] = 0; // #24
        EPwm1Regs.rsvd4[3] = 0; // #25
        EPwm1Regs.HRPWR.all |= 0x0004; // #21, unknown bit?
        DISABLE_PROTECTED_REGISTER_WRITE_MODE;
        state = 2;
        break;
	case 2:
        ENABLE_PROTECTED_REGISTER_WRITE_MODE;
        if(0 == EPwm1Regs.HRPWR.all & 0x10) {
            EPwm1Regs.HRPWR.all &= ~(0x0004);
            state = 3;
            x21 = EPwm1Regs.rsvd4[2]; // #24 (approx 35064)
        }

        DISABLE_PROTECTED_REGISTER_WRITE_MODE
	    break;
	case 3:
        ENABLE_PROTECTED_REGISTER_WRITE_MODE;
        if(x25 == 0)
            EPwm1Regs.rsvd4[0] = 0xc8;  // #22
        else {
            // setc sxm
            ePWM[0] = x25 << 1;
        }
        DISABLE_PROTECTED_REGISTER_WRITE_MODE
	case 4:
	    break;
	case 5:
	    break;
	}
	return retVal;
}
