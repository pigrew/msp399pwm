#include <stdint.h>

#include "F2802x_Device.h"     // F2802x Headerfile Include File
#include "f2802x_globalprototypes.h"
#include "cpu.h"
#include "sfo_v6.h"

// Modified implementation. Much dead code removed.
// No need for the ePWM registers, anymore.


// page 1e
static uint32_t HR_Result24a; // x26/x35
static uint32_t HR_Result24b; // x27/x36
static int state = 0;         // x29/x38
#define AVGCOUNT (8)
static uint16_t avg_i;
#define LEN_A (0x50)
// Too large of a value could cause an overfow.
#define LEN_B (0xc8)
// A measurement is performed for these two counts. The slope delta(1/Y)/delta(LEN) seems to be proportional to the MEP count.
// The result has some quantization error (or something like that). It
// seems that values which are multiples of 20 are all "in phase" with
// eachother.
// 40 (0x28) may be a good low value.
// It's unclear what the best values would be, so we'll leave it
// with the TI defaults...

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
    float a,b,c,d;
    int retVal = SFO_INCOMPLETE;
	switch(state) {
	case 0: // Initialization
	    ENABLE_PROTECTED_REGISTER_WRITE_MODE;
		EPwm1Regs.HRPWR.all = 0;
		EPwm1Regs.HRPWR.all |= 0x0008; // unknown bits...
		EPwm1Regs.HRPWR.all |= 0x0020;
		DISABLE_PROTECTED_REGISTER_WRITE_MODE;
        HR_Result24a = 0;
        HR_Result24b = 0;
        avg_i = AVGCOUNT;
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
            HR_Result24a += EPwm1Regs.rsvd4[2];
            avg_i--;
            if(avg_i!=0)
                state = 1;
            else {
                state = 3;
                avg_i = AVGCOUNT;
            }
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
        state = 4;

	case 4: // Finish measurement B
	    ENABLE_PROTECTED_REGISTER_WRITE_MODE;
	    if (0 == (EPwm1Regs.HRPWR.all & HRPWR_DONE)) {
	        EPwm1Regs.HRPWR.all &= ~(HRPWR_MEASURE); // clear start bit
	        HR_Result24b += EPwm1Regs.rsvd4[2]; // #24
            avg_i--;
            if(avg_i!=0)
                state = 3;
            else
                state = 5;
	    }
	    DISABLE_PROTECTED_REGISTER_WRITE_MODE;
	    break;

	case 5:
	    ENABLE_PROTECTED_REGISTER_WRITE_MODE;
	    a = 1.0f / (float)HR_Result24a; // a & b need to be multiplied by AVGFACTOR
	    b = 1.0f / (float)HR_Result24b;
	    c = (b-a)*(AVGCOUNT * 65535.0f); // Rsvd[4] always equals 0xFFFF at this point.
	    d = (2.0f*(LEN_B-LEN_A))/c;
        MEP_ScaleFactor_16 = (uint16_t)(d*256.0+0.5f);
        MEP_ScaleFactor = (uint16_t)(d + 0.5f);
        if(MEP_ScaleFactor > 0xFF) {
	        retVal = SFO_ERROR;
	    } else {
	        EPwm1Regs.HRMSTEP = MEP_ScaleFactor;
	        retVal = SFO_COMPLETE;
	    }
        DISABLE_PROTECTED_REGISTER_WRITE_MODE;
        state = 1;
        HR_Result24a = 0;
        HR_Result24b = 0;
        avg_i = AVGCOUNT;
        GpioDataRegs.GPATOGGLE.bit.GPIO3 = 1;
	    break;
	}
	return retVal;
}
