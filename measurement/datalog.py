#!/usr/bin/python3 import visa

import visa
import signal
import time
import numpy
import measConfig
import msppwm

from datetime import datetime

logfname = "out.csv"

#flist = [10e3]# [10e3,20e3]#,30e3,40e3,50e3,60e3,70e3,80e3]
f = 10e3
r = 3053247871

catch_count=0
default_handler = signal.getsignal(signal.SIGINT)

def signal_handler(sig, frame):
    global default_handler, catch_count
    catch_count += 1
    print ('wait:', catch_count)
    if catch_count > 3:
        # recover handler for signal.SIGINT
        signal.signal(signal.SIGINT, default_handler)
        print('expecting KeyboardInterrupt')
	
signal.signal(signal.SIGINT, signal_handler)

logf = open(logfname,"a")
logf.write("date,freq,period,ratio,v,temp_l,temp_r\n")

rm = visa.ResourceManager('C:\\windows\\system32\\visa64.dll')
pwm = msppwm.msppwm(rm,measConfig.measConfig['msppwmAddr'])
inst = rm.open_resource(measConfig.measConfig['dmmAddr'])
print(inst.query("*IDN?"))
inst.write("CONF:VOLT:DC 10")
inst.write("SENSE:ZERO:AUTO ON")
inst.write("VOLT:DC:NPLC 100")
p=round(38400.0*10000.0/f)
rr = round(r)
pwm.setPeriod(p)
pwm.setRatio(rr)
while True:
    if(catch_count > 0):
        break
    time.sleep(8)
    vmeas = inst.query("MEAS?").strip()
    t = pwm.getTemps()
    print("{},{},{},{},{},{}".format(f,p,rr,vmeas,t['l'],t['r']))
    logf.write("{},{},{},{},{},{},{}\n".format(datetime.utcnow().isoformat(),f,p,rr,vmeas,t['l'],t['r']))
    logf.flush()
logf.close()
pwm.close()
inst.close()
