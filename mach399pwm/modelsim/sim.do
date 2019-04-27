vlog ../src/*.sv ../src/*.v

vsim -L machxo2_vlg -L PMI_work work.tb_top
