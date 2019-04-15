/*
 * uart.c
 *
 *  Created on: Apr 8, 2019
 *      Author: nconrad
 */

#include <stdlib.h>
#include <stdbool.h>

#include "F2802x_Device.h"     // F2802x Headerfile Include File
#include "main.h"

#include "common/include/clk.h"
#include "common/include/gpio.h"
#include "common/include/pie.h"
#include "common/include/sci.h"

#include "ring_buffer.h"
#include "uart.h"

#define PIE_GROUP_SCIA (PIE_GroupNumber_9)
#define TXBUF_BITS 5

static SCI_Handle mySci;

static void scia_fifo_init();
__interrupt void sciaTxFifoIsr(void);
__interrupt void sciaRxFifoIsr(void);

void error(void);

volatile bool cmdComplete = false;
volatile size_t rxBufLen = 0;
volatile RB_ELM_TYPE rxbuf[RXBUF_SIZE+1]; // leave a spot for a null terminator

volatile bool tx_active = false;
volatile RB_ELM_TYPE txbuf[1<<TXBUF_BITS];
struct ring_buffer tx_rb;



// Uses RX=GPIO28:SCIRXDA:pin48 , TX=GPIO29:SCITXDA:pin1

void uart_init() {

    tx_rb.head = 0;
    tx_rb.tail = 0;
    tx_rb.buf = txbuf;
    tx_rb.n_bits = TXBUF_BITS;

    ENABLE_PROTECTED_REGISTER_WRITE_MODE;
    GpioCtrlRegs.GPAQSEL2.bit.GPIO28 = GPIO_Qual_ASync;
    GpioCtrlRegs.GPAPUD.bit.GPIO28 = GPIO_PullUp_Enable;
    GpioCtrlRegs.GPAPUD.bit.GPIO29 = GPIO_PullUp_Disable;
    GpioCtrlRegs.GPAMUX2.bit.GPIO28 = GPIO_28_Mode_SCIRXDA;
    GpioCtrlRegs.GPAMUX2.bit.GPIO29 = GPIO_29_Mode_SCITXDA;
    DISABLE_PROTECTED_REGISTER_WRITE_MODE;

    mySci = SCI_init((void *)SCIA_BASE_ADDR, sizeof(SCI_Obj));

    CLK_enableSciaClock(myClk);

    SCI_disableParity(mySci);
    SCI_setNumStopBits(mySci, SCI_NumStopBits_One);
    SCI_setCharLength(mySci, SCI_CharLength_8_Bits);

    // 57600 ???
#if (CPU_FRQ_60MHZ)
    SCI_setBaudRate(mySci, (SCI_BaudRate_e)194);
#elif (CPU_FRQ_50MHZ)
    SCI_setBaudRate(mySci, (SCI_BaudRate_e)26);
#elif (CPU_FRQ_40MHZ)
    SCI_setBaudRate(mySci, (SCI_BaudRate_e)129);
#elif (CPU_FRQ_42p5MHZ)
    SCI_setBaudRate(mySci, (SCI_BaudRate_e)30);
#else
#error Unknown CPU freq for UART
#endif

    SCI_disableTxFifoInt(mySci);
    SCI_disableRxFifoInt(mySci);
    SCI_enableTx(mySci);
    SCI_enableRx(mySci);

    scia_fifo_init();

    ENABLE_PROTECTED_REGISTER_WRITE_MODE;
    //PieVectTable.SCIRXINTA = &sciaRxFifoIsr;
    ((PIE_Obj *)myPie)->SCIRXINTA = &sciaRxFifoIsr;
    //PieVectTable.SCITXINTA = &sciaTxFifoIsr;
    ((PIE_Obj *)myPie)->SCITXINTA = &sciaTxFifoIsr;
    DISABLE_PROTECTED_REGISTER_WRITE_MODE;
    //
    // Driverlib code:
    //
  /*  PIE_registerPieIntHandler(myPie, PIE_GROUP_SCIA, (PIE_SubGroupNumber_e)PIE_InterruptSource_SCIARX,
                              (intVec_t)&sciaRxFifoIsr);
    PIE_registerPieIntHandler(myPie, PIE_GROUP_SCIA, (PIE_SubGroupNumber_e)PIE_InterruptSource_SCIATX,
                              (intVec_t)&sciaTxFifoIsr);*/
    PIE_enableInt(myPie, PIE_GroupNumber_9, PIE_InterruptSource_SCIARX);
    PIE_enableInt(myPie, PIE_GroupNumber_9, PIE_InterruptSource_SCIATX);
    SCI_enable(mySci);
}

static void scia_fifo_init()
{
    SCI_enableFifoEnh(mySci);
    SCI_resetTxFifo(mySci);
    SCI_clearTxFifoInt(mySci);
    SCI_resetChannels(mySci);
    SCI_setTxFifoIntLevel(mySci, SCI_FifoLevel_Empty);

    SCI_resetRxFifo(mySci);
    SCI_clearRxFifoInt(mySci);
    SCI_setRxFifoIntLevel(mySci, SCI_FifoLevel_1_Word);
    SCI_enableRxFifoInt(mySci);

    return;
}
static char baseFourChar(uint8_t baseFour) {
    baseFour = baseFour & 0x0F;
    if(baseFour < 10)
        return '0' + baseFour;
    return 'A' + baseFour - 10;
}

void u16hex(uint32_t value, char* result, uint16_t bits) {
    uint16_t i = 0;
    for( ; bits>0 ; bits-=4) {
        result[i++] = baseFourChar(value >> (bits-4));
    }
    return;
}
// Max is 65,536
uint16_t u16dec(uint32_t value, char* result) {
    uint32_t r;
    r=value/10000;
    result[0] = r + '0';
    value -=10000*r;

    r=value/1000;
    result[1] = r + '0';
    value -=1000*r;

    r=value/100;
    result[2] = r + '0';
    value -=100*r;

    r=value/10;
    result[3] = r + '0';
    value -=10*r;

    result[4] = value + '0';

    return 5;
}
// 0 for success
uint16_t uart_putc(uint16_t c) {
    uint16_t r = rb_put(&tx_rb, c);
    if(!tx_active) { // data can be lost here, if the buffer overflowed.
        tx_active = true;
        if(0 == rb_get(&tx_rb, &c))
            SCI_putDataNonBlocking(mySci, c);
        SCI_enableTxFifoInt(mySci);
    }
    return r;
}

// 0 on success
uint16_t uart_write(char *data, size_t len) {
    uint16_t r = 0;
    uint16_t i;
    for(i=0; i<len; i++) {
        if(rb_put(&tx_rb, data[i])) {
            r = 1;
            break; // give up on overflow
        }
    }
    if(!tx_active) { // data can be lost here, if the buffer overflowed.
        uint16_t c;
        tx_active = true;
        if(0 == rb_get(&tx_rb, &c))
            SCI_putData(mySci, c);
        SCI_enableTxFifoInt(mySci);
    }
    return r;
}

//
// sciaTxFifoIsr -
//
__interrupt void sciaTxFifoIsr(void)
{
    RB_ELM_TYPE r, d;


    // Weirdly, this ISR seems to be called before the the buffer is ready (but the FIFO is empty).
    // This will cause the ISR to be called repeatedly.
    while(tx_active && SCI_isTxReady(mySci)) {
        r = rb_get(&tx_rb, &d);
        if(r == 0) { // success
            if(!SCI_putDataNonBlocking(mySci, d)) {
                error();
            }
        } else {
            tx_active = false;
            SCI_disableTxFifoInt(mySci);
        }
    }

    SCI_clearTxFifoInt(mySci);
    PIE_clearInt(myPie, PIE_GROUP_SCIA);

    return;
}

//
// sciaRxFifoIsr -
//
__interrupt void sciaRxFifoIsr(void) {
    RB_ELM_TYPE d;
    //bool overflowed = false;

    while(SCI_getRxFifoStatus(mySci) != SCI_FifoStatus_Empty) {
        d = SCI_getData(mySci);
        if(cmdComplete) // disregard if previous CMD has not been handled
            continue;

        if(rxBufLen < RXBUF_SIZE)
            rxbuf[rxBufLen++] = d;
        else
            rxBufLen = 0; /* Overflow, clear buffer */

        if(d == '\n' || d == '\r') {
            if(rxBufLen == 1) {
                rxBufLen = 0;
            } else {
                // Line complete
                cmdComplete = true;
            }
        }
    }

    SCI_clearRxFifoOvf(mySci);
    SCI_clearRxFifoInt(mySci);
    PIE_clearInt(myPie, PIE_GROUP_SCIA);

    return;
}
