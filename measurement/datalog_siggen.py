#!/usr/bin/python3 import visa

import visa
import signal
import time
import numpy
import hp3458a
import hp33500b
from datetime import datetime

basefname = '09-dual-150k-100nplc-0phase-diff'

logfname = "out/{}_log.csv".format(basefname)
sumfname = "out/{}_sum.csv".format(basefname)

flist = [100, 300, 500, 700, 1000, 2e3, 3e3, 6e3, 10e3, 20e3,30e3]

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

def interruptableSleep(secs):
    while(secs > 0.9):
        time.sleep(1.0)
        secs = secs - 1.0
        if(catch_count > 0):
            return
    time.sleep(secs)

logf = open(logfname,"a")
logf.write("date,freq,elapsed,v\n")

sumf = open(sumfname, "a")
sumf.write("date,f,v_mean,v_std\n")

rm = visa.ResourceManager('C:\\windows\\system32\\visa64.dll')
dmm = hp3458a.hp3458a(rm, "GPIB0::5::INSTR")
siggen = hp33500b.hp33500b(rm, 'USB0::0x0957::0x2C07::MY57801028::INSTR')
while True:
    if(catch_count > 0):
        break
    for f in flist:
        if(catch_count > 0):
            break
        vlist = []
        siggen.set_f(f)
        print("Setting freq to {}".format(f))
        interruptableSleep(40)
        startTime = time.perf_counter()
        while((time.perf_counter() - startTime) < 300):
            if(catch_count > 0):
                break
            vmeas = dmm.read_v().strip()
            vlist.append(float(vmeas))
            eTime = (time.perf_counter() - startTime)
            print("{},{}".format(eTime,vmeas))
            logf.write("{},{},{},{}\n".format(datetime.utcnow().isoformat(),
                       f, eTime,  vmeas))
            logf.flush()
        print("SUM:{},{},{},{}\n".format(datetime.utcnow().isoformat(),
              f,numpy.mean(vlist),numpy.std(vlist)))
        sumf.write("{},{},{},{}\n".format(datetime.utcnow().isoformat(), f,
                   numpy.mean(vlist),numpy.std(vlist)))
        sumf.flush()
logf.close()
sumf.close()
dmm.close()
siggen.close()
