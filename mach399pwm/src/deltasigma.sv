
module deltasigma
#(
	parameter BITS=5
)
(
	input wire clk,
	input wire rst,
	
	input wire [BITS-1:0] data_in,
	//input wire data_in_en,
	
	input wire next,
	
	output wire out
);

reg [BITS+1:0] sigma, sigma_next;
//reg [BITS-1:0] dacIn, dacIn_next;
// http://www.ti.com/lit/an/slyt076/slyt076.pdf
assign out = sigma[BITS+1];
wire [BITS+1:0] delta = out? ({2'b11,{BITS{1'b0}}}) : '0;
wire [BITS+1:0] delta_out = {2'b00,data_in}  + delta;
wire [BITS+1:0] sigma_out = delta_out + sigma;

always_ff @(posedge clk or posedge rst) begin
	if(rst) begin
		//dacIn <= '0;
		//sigma <= {2'b10,{(BITS){1'b0}}};
		sigma <= '0; // Setting to above is cleaner, but harder for FPGA? This will create a small startup transient.
	end else begin
		sigma <= sigma_next;
		//dacIn <= dacIn_next;
	end
end

always_comb begin
	//dacIn_next = dacIn;
	sigma_next = sigma;
	if(next) begin
		sigma_next = sigma_out;
	end
/*	if(data_in_en) begin
		dacIn_next = data_in;
	end*/
end

endmodule