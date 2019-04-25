# Synopsys, Inc. constraint file
# C:/Users/nconrad/Documents/Lattice/PicoPWM/picopwm.sdc
# Written on Mon Apr 22 11:18:37 2019
# by Synplify Pro (R), M-2017.03L-SP1-1 Scope Editor

#
# Collections
#

#
# Clocks
#
create_clock   {n:__.CLKOP} -name {n:__.CLKOP}  -period 6.944 -freq 144 
create_clock   [get_ports "clk_USB"]   -name {n:clk_USB}     -period 83.3 -freq 12 

#
# Clock to Clock
#

#
# Inputs/Outputs
#

#
# Registers
#

#
# Delay Paths
#

#
# Attributes
#

#
# I/O Standards
#

#
# Compile Points
#

#
# Other
#