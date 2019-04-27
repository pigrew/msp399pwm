
# . compilelibs.sh  -sim_path . -lang verilog -device machxo2 -target_path machxo2/

export FOUNDRY=/c/lscc/diamond/3.10_x64/ispfpga

export PATH=$PATH:/c/lscc/diamond/tcltk/bin

tclsh $FOUNDARY/../cae_library/simulation/script/cmpl_libs.tcl $*