# -*- coding: utf-8 -*-
"""
Created on Sun Mar 31 14:37:48 2019

@author: nconrad
"""
import visa

class hp33500b:
    def __init__(self, rm: visa.ResourceManager, addr: str):
        self.dev = rm.open_resource(addr)
        self.dev.clear()
        
        self.idn = self.dev.query("*IDN?")
        assert(',335' in self.idn)
        
    def set_f(self, f):
        self.dev.write("FREQ {}".format(f))
        self.dev.query("*OPC?")
        self.dev.write("PHAS:SYNC")
    def close(self):
        self.dev.close()