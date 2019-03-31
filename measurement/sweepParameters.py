#!/usr/bin/python3 import visa

import visa
import signal
import time
import numpy
import measConfig
import msppwm
import hp34401a

from datetime import datetime

logfname = "noise_sweep3.csv"
msg="lowDriveStr/10n/intOsc"
flist = [10e3, 20e3,30e3,40e3]#,50e3,60e3,70e3,80e3]
flist = [15e3]
r_range = 200000

rmin = 3052222982 - r_range/2
rmax = rmin + r_range
rsteps = 14

rsteps=2
rlist = numpy.linspace(rmin,rmax,rsteps)

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
logf.write("msg,date,freq,period,ratio,v,temp_l,temp_r\n")

sumf = open("sum3.csv", "a")
sumf.write("msg,date,freq,period,ratio,v_mean,v_std,temp_l,temp_r\n")

rm = visa.ResourceManager('C:\\windows\\system32\\visa64.dll')
#pwm = msppwm.msppwm(rm,measConfig.measConfig['msppwmAddr'])
dmm = hp34401a.hp34401a(rm,measConfig.measConfig['dmmAddr'])

def interruptableSleep(secs):
    while(secs > 0.9):
        time.sleep(1.0)
        secs = secs - 1.0
        if(catch_count > 0):
            return
    time.sleep(secs)
t={'l':0, 'r': 0}
while True:
    if(catch_count > 0):
        break
    
    for f in flist:
        if(catch_count > 0):
            break
        p = round(38400.0*10000.0/f)
        print("setting p to {}".format(p))
        #pwm.setPeriod(p)
        
        for r in rlist:
            if(catch_count > 0):
                break
            #sio.write("r{}\n".format(r))
            rr = round(r)
            #pwm.setRatio(rr)
            interruptableSleep(2)
            vlist = []
            for _ in range(10):
                if(catch_count > 0):
                    break
                vmeas = dmm.read_v().strip()
                vlist.append(float(vmeas))
                #t = pwm.getTemps()
                print("{},{},{},{},{},{}".format(f,p,rr,vmeas,t['l'],t['r']))
                logf.write("{},{},{},{},{},{},{},{}\n".format(msg,datetime.utcnow().isoformat(),f,p,rr,vmeas,t['l'],t['r']))
                logf.flush()
            sumf.write("{},{},{},{},{},{},{},{},{}\n".format(msg,datetime.utcnow().isoformat(),f,p,rr,numpy.mean(vlist),numpy.std(vlist),t['l'],t['r']))
            print("SUM:{},{},{},{},{},{},{},{}\n".format(datetime.utcnow().isoformat(),f,p,rr,numpy.mean(vlist),numpy.std(vlist),t['l'],t['r']))
            sumf.flush()
logf.close()
sumf.close()
#pwm.close()
dmm.close()
