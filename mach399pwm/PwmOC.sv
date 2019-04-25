module pwmOC
#(
parameter WIDTH=17
)
(
input wire clk,
input wire rst,
input wire [WIDTH-1:0] tb,
output reg pwmI,
output reg pwmQ,
input wire [WIDTH:0] cmpL,
input wire [WIDTH-1:0] cmpH
);

/*reg [WIDTH-1:0] cmpH, cmpH_next;
reg [WIDTH:0] cmpL, cmpL_next;*/

reg pwmI_next, pwmQ_next;
always_ff @(posedge clk, posedge rst) begin
	if(rst) begin
		//cmpH <= '0;
		//cmpL <= 'h7;
	end else begin
		pwmI <= pwmI_next;
		pwmQ <= pwmQ_next;
		//cmpH <= cmpH_next;
		//cmpL <= cmpL_next;
	end
end

always_comb begin
	pwmI_next = pwmI & pwmQ;
	pwmQ_next = pwmI & pwmQ;
	//cmpH_next = cmpH;
	//cmpL_next = cmpL;
	if(tb==cmpH) begin
		pwmI_next = 1'b1;
		pwmQ_next = 1'b1;
	end else if (tb == cmpL[WIDTH:1]) begin
		pwmI_next = cmpL[0];
		pwmQ_next = 1'b0;
	end
end

endmodule