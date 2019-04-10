/*
 * systick.c
 *
 *  Created on: Apr 9, 2019
 *      Author: nconrad
 */

#include "F2802x_Device.h"     // F2802x Headerfile Include File
#include "systick.h"
#include "common/include/gpio.h"
#include "main.h"


static volatile uint16_t systick;
static __interrupt void cpu_timer2_isr(void);

// Use cpu timer 2
void systick_init() {
    StopCpuTimer2();

    ENABLE_PROTECTED_REGISTER_WRITE_MODE;
    PieVectTable.TINT2 = &cpu_timer2_isr;
    DISABLE_PROTECTED_REGISTER_WRITE_MODE;
   // 100 Hz???
    CpuTimer2Regs.PRD.all  = 100ul;
    // prescale by 5000 (0x1338)
    CpuTimer2Regs.TPR.all  = 0x0038; // low half is low byte
    CpuTimer2Regs.TPRH.all = 0x0013; // low half is high byte

    CpuTimer2Regs.TCR.bit.TIE = 1; // Enable interrupt

//    CpuTimer2Regs.TCR |= Cpu;
    IER |= M_INT14; // Timer 2 => Int14
    StartCpuTimer2();
}

uint16_t systick_get() {
    return systick;
}

static __interrupt void cpu_timer2_isr(void)
{
    CpuTimer2Regs.TCR.bit.TIF = 1; // Ack the timer?
    systick++;
}
