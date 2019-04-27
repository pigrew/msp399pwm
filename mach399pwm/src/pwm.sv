module pwm
#(
parameter WIDTH = 20, // width of CMPA, which is prd_bits + HRBITS + 1 (dual bit)
parameter HRBITS = 3,
parameter PERIOD = 'hff
// And one dual bit
)
(
input wire clk,
input wire rst,
input wire [WIDTH-1:0]cmpA,
output wire [(1<<HRBITS)-1:0] pwm0D,
output wire [(1<<HRBITS)-1:0] pwm1D,
output wire tb_dbg
);

reg [WIDTH-HRBITS-2:0] tb;
reg [WIDTH-HRBITS-2:0] tb_next;

reg [WIDTH-HRBITS-2:0] prd, prd_next;  // PRD is this number, plus 1

reg [WIDTH-HRBITS-2:0] midPrd;
reg [WIDTH-2:0] cmpL1;

pwmOC #(.WIDTH(WIDTH), .HRBITS(3)) OC0(.clk(clk), .rst(rst), .tb(tb), .pwmD(pwm0D),
	.cmpH('0),
	.cmpL(cmpA[WIDTH-1:1]));

pwmOC #(.WIDTH(WIDTH), .HRBITS(3)) OC1(.clk(clk), .rst(rst), .tb(tb), .pwmD(pwm1D), .cmpH( {{HRBITS{1'b0}},midPrd}<<HRBITS),
	.cmpL(cmpL1));
	
assign tb_dbg = tb[5];
always_comb begin 
	midPrd = (prd>>1) + 'd1;
	cmpL1 = ({midPrd,{HRBITS{1'b0}}})+cmpA[WIDTH-1:1]+cmpA[0];
	if(cmpL1 > {prd,{HRBITS{1'b0}}}) begin
		cmpL1 = cmpL1 - ({prd,{HRBITS{1'b0}}});
	end
end
always_ff @(posedge clk, posedge rst) begin
	if(rst) begin
		prd <= PERIOD;
		tb <= 1'b0;
	end else begin
		tb <= tb_next;
		prd <= prd_next;
	end
end

always_comb begin
	tb_next = tb+1;
	prd_next = prd;
	if(tb==prd) begin
		tb_next = '0;
	end
end

endmodule