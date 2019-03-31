# -*- coding: utf-8 -*-
"""
Spyder Editor

This is a temporary script file.
"""

import hp3458a
import visa

rm = visa.ResourceManager('C:\\windows\\system32\\visa64.dll')

dmm = hp3458a.hp3458a(rm, "GPIB0::5::INSTR")

print(dmm.read_v())
dmm.Close()
