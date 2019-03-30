# -*- coding: utf-8 -*-
"""
Created on Sat Mar 30 16:23:35 2019

@author: nconrad
"""
#import visa

class msppwm:
    def __init__(self, rm, addr):
        self.addr = addr
        self.inst = rm.open_resource(addr)
        self.inst.baud_rate = 57600
        self.inst.data_bits = 8
        self.inst.query("t?") # dummy query to clear buffer
        
    def setRatio(self, ratio): # Ratio should be an int
        x = self.inst.query("r{}\n".format(round(ratio))).strip()
        assert(x == "OK")
        
    def setPeriod(self, p): # Period should be an int
        x = self.inst.query("p{}\n".format(round(p))).strip()
        assert(x == "OK")
        
    def close(self):
        self.inst.close()