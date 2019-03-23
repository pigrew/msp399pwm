/*
 * tmp411.c
 *
 *  Created on: Mar 22, 2019
 *      Author: nconrad
 */

#define TMP411_ADDR 0x4c

#include <stdlib.h>
#include "driverlib.h"

static uint8_t tmp411_readReg(uint8_t addr);
static uint16_t tmp411_readRegWord(uint8_t addr);
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

    USCI_B_I2C_initMaster(USCI_B0_BASE, &param);
    USCI_B_I2C_setSlaveAddress(USCI_B0_BASE,TMP411_ADDR);
    USCI_B_I2C_enable(USCI_B0_BASE);

    tmp411_writeReg(TMP411_REG_RESET_WR,0x00);
    tmp411_writeReg(TMP411_REG_RES,TMP411_REG_RES_12);
    tmp411_writeReg(TMP411_REG_RATE_WR,TMP411_REG_RATE_0p5);
}
static uint8_t tmp411_readReg(uint8_t addr) {
    uint8_t x;

    USCI_B_I2C_setMode(USCI_B0_BASE,USCI_B_I2C_TRANSMIT_MODE);
    USCI_B_I2C_masterSendSingleByte(USCI_B0_BASE, addr);
    USCI_B_I2C_setMode(USCI_B0_BASE,USCI_B_I2C_RECEIVE_MODE);
    USCI_B_I2C_masterReceiveSingleStart(USCI_B0_BASE);
    x = USCI_B_I2C_masterReceiveSingle(USCI_B0_BASE);

    return x;
}
static uint16_t tmp411_readRegWord(uint8_t addr) {
    uint16_t x;

    USCI_B_I2C_masterSendSingleByte(USCI_B0_BASE, addr);

    USCI_B_I2C_masterReceiveMultiByteStart(USCI_B0_BASE);

    //Wait for RX buffer
    while (!(HWREG8(USCI_B0_BASE + OFS_UCBxIFG) & UCRXIFG));

    x = USCI_B_I2C_masterReceiveMultiByteNext(USCI_B0_BASE) << 8;

    //Wait for RX buffer
    while (!(HWREG8(USCI_B0_BASE + OFS_UCBxIFG) & UCRXIFG));

    x = x | USCI_B_I2C_masterReceiveMultiByteFinish(USCI_B0_BASE);
    return x;
}
static void tmp411_writeReg(uint8_t addr, uint8_t data) {

    USCI_B_I2C_setMode(USCI_B0_BASE,USCI_B_I2C_TRANSMIT_MODE);
    USCI_B_I2C_masterSendMultiByteStart(USCI_B0_BASE, addr);
    USCI_B_I2C_masterSendMultiByteFinish(USCI_B0_BASE, data);

    return;
}
uint16_t tmp411_getLocal() {
    return tmp411_readRegWord(TMP411_REG_LT_HIGH_RD);
}
uint16_t tmp411_getRemote() {
    return tmp411_readRegWord(TMP411_REG_RT_HIGH_RD);
}
