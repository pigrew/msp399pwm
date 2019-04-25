/*
 * asmfuncs.c
 *
 *  Created on: Apr 18, 2019
 *      Author: nconrad
 */


#include "F2802x_Device.h"     // F2802x Headerfile Include File
#include "main.h"

#pragma CODE_SECTION(asm_idle, "ramfuncs")
void asm_idle() {
    ENABLE_PROTECTED_REGISTER_WRITE_MODE;
    FlashRegs.FPWR.bit.PWR = 0; // Go to low-power flash state
    DISABLE_PROTECTED_REGISTER_WRITE_MODE;
    asm(" IDLE");
}


void asm_estop0() {
    asm(" ESTOP0");
}
