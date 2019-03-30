/*
 * usci_i2c.c
 *
 *  Created on: Mar 29, 2019
 *      Author: nconrad
 */

// Mostly blocking library for I2C UCSI B (tested on MSP430F5xx)

// Derived from the TI SLAA208c example code for an EEPROM
// Influenced by https://github.com/jwr/msp430_usi_i2c
// MSP430Ware_3_80_07_00/examples/devices/MSP430F5xx_6xx/
//       MSP430F51x2_MSP430F51x1_Code_Examples/C/MSP430F51x2_usci_i2c_standard_master.c

#include <stdint.h>
#include "msp430.h"
#include "usci_i2c.h"

#ifdef __MSP430F5XX_6XX_FAMILY__
#include "msp430f5xx_6xxgeneric.h"
#else
#error Only above families are supported!
#endif

//*****************************************************************************
//
// Macros for hardware access

//
//*****************************************************************************


#define HWREG8(x)         (*((volatile uint8_t *)(x)))
#define HWREG16(x)        (*((volatile uint16_t *)((uint16_t)x)))
//#define HWREG16(x)      *((volatile uint16_t *)(x)))
#define HWREG8_L(x)       (*((volatile uint8_t *)((uint8_t *)&x)))
#define HWREG8_H(x)       (*((volatile uint8_t *)(((uint8_t *)&x)+1)))
#define HWREG16_L(x)      (*((volatile uint16_t *)((uint16_t *)&x)))
#define HWREG16_H(x)      (*((volatile uint16_t *)(((uint16_t *)&x)+1)))

#define UCB_CTLW0 (HWREG16(base + OFS_UCBxCTLW0))
#define UCB_CTL0  (HWREG8(base + OFS_UCBxCTL0))
#define UCB_CTL1  (HWREG8(base + OFS_UCBxCTL1))
#define UCB_BRW   (HWREG16(base + OFS_UCBxBRW))
#define UCB_STAT  (HWREG8(base + OFS_UCBxCTL0))
#define UCB_RXBUF (HWREG8(base + OFS_UCBxRXBUF))
#define UCB_TXBUF (HWREG8(base + OFS_UCBxTXBUF))
#define UCB_I2COA (HWREG16(base + OFS_UCBxI2COA))
#define UCB_I2CSA (HWREG16(base + OFS_UCBxI2CSA))
#define UCB_ICTL  (HWREG16(base + OFS_UCBxICTL))
#define UCB_IE    (HWREG8(base + OFS_UCBxIE))
#define UCB_IFG   (HWREG8(base + OFS_UCBxIFG))
#define UCB_IV    (HWREG16(base + OFS_UCBxIV))

#define PORT_HWREG16(base,REG) (HWREG16(base + OFS_PA##REG))

static uint16_t base = 0x0000;

// Initialize USCI as master
// The GPIO registers are different for different models
void ucb_i2c_init(
        uint16_t ucb_i2c_base, uint16_t port_base,
        uint8_t sda_pin, uint8_t scl_pin, // Pin # (not bit field)
        uint16_t clk_source, // UCSSEL__UCLK, UCSSEL__ACLK , UCSSEL__SMCLK
        uint16_t clock_div) {

    base = ucb_i2c_base;

    // Recommended initialisation steps of I2C module as shown in User Guide:
    UCB_CTL1 |= UCSWRST;                     // Enable SW reset
    UCB_CTL0 = UCMST + UCMODE_3 + UCSYNC;    // I2C Master, synchronous mode, 7-bit addresses
    UCB_CTL1 = clk_source | UCTR | UCSWRST;  // Use SMCLK, TX mode, keep SW reset
    UCB_BRW = clock_div;                     // fSCL = SMCLK/12 = ~100kHz
    UCB_IE = 0x00;                           // Disable interrupts
    // UCB0I2COA = 0x01A5;                   // own address. Useful for multi-master?

#ifdef __MSP430_HAS_PASEL0__
#error Not sure which pins to set?
#else
    PORT_HWREG16(port_base,SEL) |= (1 << (sda_pin)) | (1 << (scl_pin));
#endif
    UCB_CTL1 &= ~UCSWRST;                    // Clear SW reset, resume operation

    if (UCB_STAT & UCBBUSY) // test if bus to be free
    {                                           // otherwise a manual Clock on is
                                                // generated
        PORT_HWREG16(port_base,SEL) &= ~(1 << (scl_pin));               // Select Port function for SCL
        PORT_HWREG16(port_base,OUT) &= ~(1 << (scl_pin));               //
        PORT_HWREG16(port_base,DIR) |= (1 << (scl_pin));                // drive SCL low
        PORT_HWREG16(port_base,SEL) |= (1 << (sda_pin)) | (1 << (scl_pin));      // select module function for the
                                                // used I2C pins
  };
}

// Uses repeated start to read a two-byte value.
// The first returned byte will be shifted to the HIGH byte.
uint16_t ucb_i2c_readReg16(uint16_t addr, uint8_t reg) {
    uint16_t x;

    UCB_I2CSA = addr;
    UCB_IFG &= ~(UCTXIFG + UCRXIFG + UCNACKIFG);       // Clear any pending interrupts

    UCB_CTL1 |= UCTR | UCTXSTT; // transmit mode, transmit start; examples set simultaneously?

    //Poll for transmit interrupt flag.
    while (!(UCB_IFG & UCTXIFG))
        ;

    //Send register number
    UCB_TXBUF = reg;

    //Poll for transmit interrupt flag.
    while (!(UCB_IFG & UCTXIFG))
        ;

    UCB_CTL1 &= ~UCTR; // Set RX Mode
    UCB_CTL1 |= UCTXSTT; // transmit start

    //Wait for RX buffer
    // No need to wait for ACK of addr?
    while (!(UCB_IFG & UCRXIFG))
        ;

    x = UCB_RXBUF;
    UCB_CTL1 |= UCTXSTP; // transmit stop
    x = x << 8; // shift MSB to upper byte

    while (!(UCB_IFG & UCRXIFG))
        ;

    x = x | UCB_RXBUF;

    while (UCB_CTL1 & UCTXSTP)
        ;
    return x;
}

// Uses repeated start to read a two-byte value.
// The first transmitted byte is the HIGH byte.
void ucb_i2c_writeReg8(uint16_t addr, uint8_t reg, uint8_t value) {

    UCB_I2CSA = addr;
    UCB_IFG &= ~(UCTXIFG + UCRXIFG + UCNACKIFG);       // Clear any pending interrupts

    UCB_CTL1 |= UCTR | UCTXSTT; // transmit mode, transmit start; examples set simultaneously?
//    UCB_CTL1 |= UCTXSTT; // transmit start

    //Poll for transmit interrupt flag.
    while (!(UCB_IFG & UCTXIFG))
        ;

    //Send register number
    UCB_TXBUF = reg;
    //Poll for transmit interrupt flag.
    while (!(UCB_IFG & UCTXIFG))
        ;
    UCB_TXBUF = value;
    //Poll for transmit interrupt flag.
    while (!(UCB_IFG & UCTXIFG))
        ;
    UCB_CTL1 |= UCTXSTP; // transmit stop
    //Poll for transmit interrupt flag.
    while (UCB_CTL1 & UCTXSTP)
        ;
    return;
}

