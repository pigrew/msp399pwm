
#include "F2802x_Device.h"     // F2802x Headerfile Include File
#include "cpu.h"
#include "sfo_v6.h"

// Modified implementation. Much dead code removed.
// No need for the ePWM registers, anymore.

extern int MEP_ScaleFactor;
float MEP_SF_float;
// page 1e
static uint16_t HR_Result24a; // x26/x35
static uint16_t HR_Result24b; // x27/x36
static int state = 0;         // x29/x38

#define LEN_A (0x50)
#define LEN_B (0xc8)
// A measurement is performed for these two counts. The slope delta(1/Y)/delta(LEN) seems to be proportional to the MEP count.

// Registers:
// HRPWR (0x021).0x0004 : Start measurement/stop measurement
//              .0x0008 :
//              .0x0010 : Measurement done?
//              .0x0020 :
//              .0x03c0 : Disable power? All four bits?
// CNT   (0x022).0x00FF : Measurement count? low 8-bits
// Res0  (0x024).0xFFFF : Result0 (actual result)
// Res1  (0x025).0xFFFF : Result1 (ends up 0xFFFF)

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
    int retVal = SFO_INCOMPLETE;
	switch(state) {
	case 0: // Initialization
	    ENABLE_PROTECTED_REGISTER_WRITE_MODE;
		EPwm1Regs.HRPWR.all = 0;
		EPwm1Regs.HRPWR.all |= 0x0008; // unknown bits...
		EPwm1Regs.HRPWR.all |= 0x0020;
		DISABLE_PROTECTED_REGISTER_WRITE_MODE;
		state = 1;
		break;
	case 1: // Start measurement A
        ENABLE_PROTECTED_REGISTER_WRITE_MODE;
        EPwm1Regs.HRPWR.all &= ~(HRPWR_MEPOFF_MASK); // #21 Enable power?
        EPwm1Regs.rsvd4[0] = LEN_A;    // #22
        EPwm1Regs.rsvd4[2] = 0; // #24
        EPwm1Regs.rsvd4[3] = 0; // #25
        EPwm1Regs.HRPWR.all |= HRPWR_MEASURE; // #21, starts a measurement
        DISABLE_PROTECTED_REGISTER_WRITE_MODE;
        state = 2;
        break;
	case 2: // Finish measurement A
        ENABLE_PROTECTED_REGISTER_WRITE_MODE;
        if(0 == (EPwm1Regs.HRPWR.all & HRPWR_DONE)) {
            EPwm1Regs.HRPWR.all &= ~(HRPWR_MEASURE); // clear start bit
            HR_Result24a = EPwm1Regs.rsvd4[2];
            state = 3;
        }
        DISABLE_PROTECTED_REGISTER_WRITE_MODE;
	    break;
	case 3: // Start measurement B
        ENABLE_PROTECTED_REGISTER_WRITE_MODE;
        EPwm1Regs.rsvd4[0] = LEN_B;
        EPwm1Regs.rsvd4[2] = 0;
        EPwm1Regs.rsvd4[3] = 0;
        EPwm1Regs.HRPWR.all |= HRPWR_MEASURE; // start
        DISABLE_PROTECTED_REGISTER_WRITE_MODE;
        /*for(i=0; i<0x32; i++) {
            asm(" nop");
            // Delay loop?????
        }*/
        state = 4;

	case 4: // Finish measurement B
	    ENABLE_PROTECTED_REGISTER_WRITE_MODE;
	    if (0 == (EPwm1Regs.HRPWR.all & HRPWR_DONE)) {
	        EPwm1Regs.HRPWR.all &= ~(HRPWR_MEASURE); // clear start bit
	        HR_Result24b = EPwm1Regs.rsvd4[2]; // #24
	        state = 5;
	    }
	    DISABLE_PROTECTED_REGISTER_WRITE_MODE;
	    break;

	case 5:
	    ENABLE_PROTECTED_REGISTER_WRITE_MODE;
	    a = 1.0f / (float)HR_Result24a;
	    b = 1.0f / (float)HR_Result24b;
	    c = (b-a)*65535.0f; // Rsvd[4] always equals 0xFFFF at this point.
	    MEP_SF_float = (2.0f*(LEN_B-LEN_A))/c;
        MEP_ScaleFactor = (uint16_t)(MEP_SF_float + 0.5f);
        if(MEP_ScaleFactor > 0xFF) {
	        retVal = SFO_ERROR;
	    } else {
	        EPwm1Regs.HRMSTEP = MEP_ScaleFactor;
	        retVal = SFO_COMPLETE;
	    }
        DISABLE_PROTECTED_REGISTER_WRITE_MODE;
        state = 1;
	    break;
	}
	return retVal;
}
