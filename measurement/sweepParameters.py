#!/usr/bin/python3 import visa

import visa
import signal
import time
import numpy
import measConfig
import msppwm

flist = [10e3,20e3]#,30e3,40e3,50e3,60e3,70e3,80e3]

rmin = 3052222982 - 50000
rmax = rmin + 100000
rsteps = 3

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

rm = visa.ResourceManager('C:\\windows\\system32\\visa64.dll')

pwm = msppwm.msppwm(rm,measConfig.measConfig['msppwmAddr'])

inst = rm.open_resource(measConfig.measConfig['dmmAddr'])
print(inst.query("*IDN?"))
inst.write("CONF:VOLT:DC 10")
inst.write("SENSE:ZERO:AUTO ON")
inst.write("VOLT:DC:NPLC 100")
for f in flist:
    p = round(38400.0*10000.0/f)
    print("setting p to {}".format(p))
    pwm.setPeriod(p)
    for r in numpy.linspace(rmin,rmax,rsteps):
        if(catch_count > 0):
            break
        #sio.write("r{}\n".format(r))
        rr = round(r)
        pwm.setRatio(rr)
        time.sleep(3)
        vmeas = inst.query("MEAS?").strip()
        print("{},{},{},{}".format(f,p,rr,vmeas))

msppwm.close()
inst.close()
