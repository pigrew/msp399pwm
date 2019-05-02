vlog -quiet ../src/*.sv ../src/*.v

vsim -L machxo2_vlg -L PMI_work work.tb_top
add wave /tb_top/DUT/pwmA/DS/*
add wave /tb_top/DUT/*
add wave /tb_top/DUT/pwmA/*
configure wave -signalnamewidth 1

run 50us