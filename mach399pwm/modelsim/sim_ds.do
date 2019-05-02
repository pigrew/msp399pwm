vlog -quiet ../src/deltasigma.sv  ../src/tb_deltasigma.sv ../src/twister.sv
vsim work.tb_deltasigma

add wave tb_deltasigma/* tb_deltasigma/DUT/*
add wave -noupdate -format Analog-Step -radix hexadecimal -min 0 -max 128 -height 74  /tb_deltasigma/DUT/sigma
configure wave -signalnamewidth 1
run 30us;
