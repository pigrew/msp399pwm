# -*- coding: utf-8 -*-
"""
Created on Sun Mar 31 14:37:48 2019

@author: nconrad
"""
import visa

class hp34401a:
    def __init__(self, rm: visa.ResourceManager, addr: str):
        self.dev = rm.open_resource(addr)
        self.dev.clear()
        
        self.idn = self.dev.query("*IDN?")
        #assert(self.idn == "HP3458A")
        
        self.dev.write("DISPLAY OFF")
        self.dev.write("CONF:VOLT:DC 10")
        self.dev.write("SENSE:ZERO:AUTO ON")
        self.dev.write("VOLT:DC:NPLC 100")

    def read_v(self):
        return self.dev.query("READ?")
    def close(self):
        self.dev.close()