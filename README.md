# msp399pwm

This project is a 10V LM399-based reference using high resolution PWM sources. In the end, using a MachXO2 was significantly better than the rest of the MCUs, and the use of MCUs was abandoned.

All of the various firmwares are included in this archive. With the FPGA, the project works relatively well. The FPGA can be controlled via I2C to adjust the output duty cycle and frequency.

# PCBs
The PCBs are not yet included in the repository, but will be once they become more final (The first revision had many bugs).
They are designed using Altium Designer 19.

# MSP430
The MSP430 firmware is built using Code Composer Studio 9.0. 

The Timer_D of the MSP430 is used due to its high speed (>=256 MHz), 16-bit resolution, and reasonably low power. The project also uses
a UART for remote control, and I2C for interacting with a temperature sensor (which a future plan of implementing temperature correction).

# MSP430 UART Control

See firmware/cmds.txt for details.

The UART control is a very simple at the moment, and can be easily confused. There is no command queue, so you must insert a delay after each
control command, until "OK" or "ERROR" is received.
