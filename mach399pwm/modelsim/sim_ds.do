vlog ../src/deltasigma.sv
vlog ../src/tb_deltasigma.sv
vsim work.tb_deltasigma

add wave tb_deltasigma/* tb_deltasigma/DUT/*
configure wave -signalnamewidth 1
run 30us;
