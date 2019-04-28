module pwmOC
#(
parameter WIDTH=17,
HRBITS=3
)
(
input wire clk,
input wire rst,
input wire [WIDTH-HRBITS-1:0] tb,
output reg [(1<<HRBITS)-1:0] pwmD,
input wire [WIDTH-1:0] cmpL,
input wire [WIDTH-1:0] cmpH
);
// Internal regs which load when cmpL is reached;
reg [WIDTH-1:0] cmpL_int, cmpL_int_next;

/*reg [WIDTH-1:0] cmpH, cmpH_next;
reg [WIDTH:0] cmpL, cmpL_next;*/
reg x, x_next;
reg [(1<<HRBITS)-1:0] pwmD_next;
always_ff @(posedge clk, posedge rst) begin
	if(rst) begin
		pwmD <= '1;
		x <= '1;
		cmpL_int <= 'd50;
		//cmpH <= '0;
		//cmpL <= 'h7;
	end else begin
		pwmD <= pwmD_next;
		x <= x_next;
		cmpL_int <= cmpL_int_next;
		//cmpH <= cmpH_next;
		//cmpL <= cmpL_next;
	end
end

always_comb begin
	x_next = x;
	cmpL_int_next = cmpL_int;
	pwmD_next = {(1<<HRBITS){x}};
	//cmpH_next = cmpH;
	//cmpL_next = cmpL;
	if(tb==cmpH[WIDTH-1:HRBITS]) begin
		x_next = '1;
		pwmD_next = {((1<<HRBITS)){1'b1}} << cmpH[HRBITS-1:0];
		cmpL_int_next = cmpL;
	end else if (tb == cmpL_int[WIDTH-1:HRBITS]) begin
		x_next = '0;
		pwmD_next = {((1<<HRBITS)){1'b1}} >> ((1<<HRBITS) - cmpL_int[HRBITS-1:0]);
	end
end

endmodule