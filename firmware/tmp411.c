/*
 * tmp411.c
 *
 *  Created on: Mar 22, 2019
 *      Author: nconrad
 */

#define TMP411_ADDR 0x4c

#include <stdlib.h>
#include "driverlib.h"

#define I2C_BASE USCI_B0_BASE

#define I2C_REG8(ofs) (HWREG8(I2C_BASE + ofs))

static uint8_t tmp411_readReg8(uint8_t addr);
static uint16_t tmp411_readReg16(uint8_t addr);
static void tmp411_writeReg(uint8_t addr, uint8_t data);

#define TMP411_REG_LT_HIGH_RD 0x00
#define TMP411_REG_LT_LOW_RD 0x15

#define TMP411_REG_RT_HIGH_RD 0x01
#define TMP411_REG_RT_LOW_RD 0x10

#define TMP411_REG_RES 0x1a
#define TMP411_REG_RES_9 (0x1c | 0x00)
#define TMP411_REG_RES_10 (0x1c | 0x01)
#define TMP411_REG_RES_11 (0x1c | 0x10)
#define TMP411_REG_RES_12 (0x1c | 0x11)

#define TMP411_REG_RATE_RD 0x04
#define TMP411_REG_RATE_WR 0x0a
#define TMP411_REG_RATE_0p25 (0x02)
#define TMP411_REG_RATE_0p5 (0x03)

#define TMP411_REG_RESET_WR 0xFC

void tmp411_init() {
    USCI_B_I2C_initMasterParam param =
    {
     .selectClockSource = USCI_B_I2C_CLOCKSOURCE_ACLK,
     .i2cClk = 1500000,
     .dataRate = USCI_B_I2C_SET_DATA_RATE_100KBPS
    };

    //Assign I2C pins to USCI_B0
    GPIO_setAsPeripheralModuleFunctionInputPin(
        GPIO_PORT_P1,
        GPIO_PIN4 + GPIO_PIN5
        );

    USCI_B_I2C_initMaster(I2C_BASE, &param);
    USCI_B_I2C_setSlaveAddress(I2C_BASE,TMP411_ADDR);
    USCI_B_I2C_enable(I2C_BASE);

    tmp411_writeReg(TMP411_REG_RESET_WR,0x00);
    tmp411_writeReg(TMP411_REG_RES,TMP411_REG_RES_12);
    tmp411_writeReg(TMP411_REG_RATE_WR,TMP411_REG_RATE_0p5);
}
static uint8_t tmp411_readReg8(uint8_t addr) {
    uint8_t x;

    USCI_B_I2C_setMode(I2C_BASE,USCI_B_I2C_TRANSMIT_MODE);
    USCI_B_I2C_masterSendSingleByte(I2C_BASE, addr);

    USCI_B_I2C_setMode(I2C_BASE,USCI_B_I2C_RECEIVE_MODE);
    USCI_B_I2C_masterReceiveSingleStart(I2C_BASE);
    x = USCI_B_I2C_masterReceiveSingle(I2C_BASE);


    return x;
}
static uint16_t tmp411_readReg16(uint8_t addr) {
    uint16_t x;

    USCI_B_I2C_setMode(I2C_BASE,USCI_B_I2C_TRANSMIT_MODE);
    USCI_B_I2C_masterSendSingleByte(I2C_BASE, addr);

    I2C_REG8(OFS_UCBxCTL1) &= ~UCTR; // Set RX Mode
    I2C_REG8(OFS_UCBxCTL1) |= UCTXSTT; // transmit start
    // No need to wait for ACK of addr?
    //Wait for RX buffer
    while (!(I2C_REG8(OFS_UCBxIFG) & UCRXIFG))
        ;

    x = I2C_REG8(OFS_UCBxRXBUF);
    I2C_REG8(OFS_UCBxCTL1) |= UCTXSTP; // transmit stop
    x = x << 8; // shift MSB to upper byte

    while (!(I2C_REG8(OFS_UCBxIFG) & UCRXIFG))
        ;

    x = x | I2C_REG8(OFS_UCBxRXBUF);
    // No need to wait for stop to be done

    return x;
}
static void tmp411_writeReg(uint8_t addr, uint8_t data) {

    USCI_B_I2C_setMode(USCI_B0_BASE,USCI_B_I2C_TRANSMIT_MODE);
    USCI_B_I2C_masterSendMultiByteStart(USCI_B0_BASE, addr);
    USCI_B_I2C_masterSendMultiByteFinish(USCI_B0_BASE, data);

    return;
}
uint16_t tmp411_getLocal() {
    //return 0x1234;
    return tmp411_readReg16(TMP411_REG_LT_HIGH_RD);
}
uint16_t tmp411_getRemote() {
    return tmp411_readReg16(TMP411_REG_RT_HIGH_RD);
}
