`timescale 1 ns / 1 ps

module tb_deltasigma();
localparam B = 5;
reg clk = '0;
reg rst = '1;

always begin
	clk = '0;
	#500ns;
	clk = '1;
	#500ns;
end

reg [B-1:0] data_in;
reg data_in_en;
reg next;
wire out;
clocking cb @(posedge clk);
	default input #10ns output #10ns;
	output negedge rst;
	output data_in, data_in_en, next;
	input out;
endclocking

initial begin
	data_in = 5'h07;
	data_in_en = '0;
	next = 0;
	@cb;
	@cb;
	cb.rst <= '0;
	@cb;
	@cb;
	cb.data_in_en <= '1;
	@cb;
	cb.data_in_en <= '0;
	@cb;
	cb.next <= '1;
end

deltasigma #(.BITS(B)) DUT(.*);

endmodule