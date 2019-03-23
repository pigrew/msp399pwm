# msp399pwm

This project is a 10V LM399-based reference using MSP430 PWM.

## PCBs
The PCBs are not yet included in the repository, but will be once they become more final (The first revision had many bugs).
They are designed using Altium Designer 19.

## MSP430
The MSP430 firmware is built using Code Composer Studio 9.0. 

The Timer_D of the MSP430 is used due to its high speed (>=256 MHz), 16-bit resolution, and reasonably low power. The project also uses
a UART for remote control, and I2C for interacting with a temperature sensor (which a future plan of implementing temperature correction).
