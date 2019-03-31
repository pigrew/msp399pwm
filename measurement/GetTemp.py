# -*- coding: utf-8 -*-
"""
Created on Sat Mar 30 16:11:56 2019

@author: nconrad
"""
import msppwm
import measConfig
import visa


rm = visa.ResourceManager('C:\\windows\\system32\\visa64.dll')

pwm = msppwm.msppwm(rm,measConfig.measConfig['msppwmAddr'])
print(pwm.getTemps())
pwm.close()