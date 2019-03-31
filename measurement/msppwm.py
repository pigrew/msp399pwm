# -*- coding: utf-8 -*-
"""
Created on Sat Mar 30 16:23:35 2019

@author: nconrad
"""
import visa
import re

class msppwm:
    def __init__(self, rm: visa.ResourceManager, addr):
        self.addr = addr
        self.inst = rm.open_resource(addr)
        self.inst.baud_rate = 57600
        self.inst.data_bits = 8
        self.inst.query("t?") # dummy query to clear buffer
        self.inst.query("t0") # Disable periodic temp report
        
    def setRatio(self, ratio): # Ratio should be an int
        x = self.inst.query("r{}\n".format(round(ratio))).strip()
        assert(x == "OK")
        
    def setPeriod(self, p): # Period should be an int
        x = self.inst.query("p{}\n".format(round(p))).strip()
        assert(x == "OK")
        
    def getTemps(self):
        x = self.inst.query("t?").strip()
        m = re.match(r'^T([0-9A-Z]+),([0-9A-Z]+)$',x)
        return {'l': int(m.group(1),16)/256.0 , 'r': int(m.group(2),16)/256.0 }
    
    def close(self):
        self.inst.close()
        