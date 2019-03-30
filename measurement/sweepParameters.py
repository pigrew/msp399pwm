#!/usr/bin/python3 import visa
import io
import serial
import visa
import signal
import sys
import time
import numpy

flist = [10e3,20e3,30e3,40e3,50e3,60e3,70e3,80e3]

rmin = 3052222982 - 50000
rmax = rmin + 100000
rsteps = 2

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

addr = 'GPIB0::22::INSTR'
serialPort = '/dev/ttyS10'


ser = serial.Serial(serialPort,57600)
#sio = io.TextIOWrapper(io.BufferedRWPair(ser, ser, 1), encoding='ascii')

rm = visa.ResourceManager('C:\\windows\\system32\\visa64.dll')
inst = rm.open_resource(addr)
print(inst.query("*IDN?"))
inst.write("CONF:VOLT:DC 10")
inst.write("SENSE:ZERO:AUTO ON")
inst.write("VOLT:DC:NPLC 100")
for f in flist:
	p = round(38400.0/10000.0/f)
	print("setting p to {}".format(p))
	ser.write(bytes("p{}\n".format(p),'ascii'))
	ser.flush()
	ser.read(size=3)
	for r in numpy.linspace(rmin,rmax,rsteps):
		if(catch_count > 0):
			break
		#sio.write("r{}\n".format(r))
		rr = round(r)
		ser.write(bytes("r{}\n".format(rr),'ascii'))
		ser.flush()
		ser.read(size=3)
		#print(sio.readline())
		time.sleep(8)
		vmeas = inst.query("MEAS?").strip()
		print("{},{},{},{}".format(f,p,rr,vmeas))
ser.close()
inst.close()
