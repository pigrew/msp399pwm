# -*- coding: utf-8 -*-
"""
Created on Sat Mar 30 16:11:56 2019

@author: nconrad
"""
import visa
import measConfig


rm = visa.ResourceManager('C:\\windows\\system32\\visa64.dll')
msppwm = rm.open_resource(measConfig.measConfig['msppwmAddr'])
msppwm.baud_rate = 57600
msppwm.data_bits = 8
t=msppwm.query("t?")

msppwm.close()