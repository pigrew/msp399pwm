# -*- coding: utf-8 -*-
"""
Created on Sun Mar 31 14:37:48 2019

@author: nconrad
"""
import visa

class hp3458a:
    def __init__(self, rm: visa.ResourceManager, addr: str):
        self.dev = rm.open_resource(addr)
        self.dev.read_termination = '\r\n'
        self.dev.write_termination = '\n'
        self.dev.clear()
        
        self.idn = self.dev.query("ID?")
        assert(self.idn == "HP3458A")
        print("id is {}".format(self.idn))
        self.dev.write("DISP MSG,\"                 \"")
        
        # Stop auto-trigger
        self.dev.write("TRIG HOLD")
        self.dev.write("NPLC 100")
        self.dev.write("NRDGS 1,LINE")
    def read_v(self):
        self.dev.write("TRIG SGL")
        return (self.dev.read())
    def close(self):
        self.dev.close()