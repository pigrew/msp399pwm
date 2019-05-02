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
wire ds_out;
wire twister_out;
wire tw_next_out;
clocking cb @(posedge clk);
	default input #100ns output #100ns;
	output negedge rst;
	output data_in, data_in_en, next;
	input ds_out, twister_out;
endclocking

initial begin
	data_in = 5'h10;
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
	repeat (20) begin
		cb.next <= '1;
		@cb;
		cb.next <= '0;
		@cb;
		@cb;
		@cb;
	end
end

deltasigma #(.BITS(B)) DUT(.out(ds_out), .next(tw_next_out), .*);
twister TWISTER(.clk, .rst, .d_in(ds_out), .d_out(twister_out), .next_out(tw_next_out), .next);

endmodule