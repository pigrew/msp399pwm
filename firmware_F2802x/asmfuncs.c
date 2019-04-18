/*
 * asmfuncs.c
 *
 *  Created on: Apr 18, 2019
 *      Author: nconrad
 */



void asm_idle() {
    asm(" IDLE");
}


void asm_estop0() {
    asm(" ESTOP0");
}
