module pwm
#(
parameter WIDTH = 20, // width of CMPA, which is prd_bits + HRBITS
parameter HRBITS = 3,
parameter PERIOD = 'hff,
parameter DSBITS = 5
// And one dual bit
)
(
input wire clk,
input wire rst,
input wire [WIDTH-1:0]cmpA,
input wire [DSBITS-1:0] ds_fraction,
output wire [(1<<HRBITS)-1:0] pwm0D,
output wire [(1<<HRBITS)-1:0] pwm1D,
output wire tb_dbg
);

reg [WIDTH-HRBITS-1:0] tb;
reg [WIDTH-HRBITS-1:0] tb_next;
reg [WIDTH-HRBITS-1:0] prd, prd_next;  // PRD is this number, plus 1

reg [WIDTH-HRBITS-1:0] midPrd;
reg [WIDTH-1:0] cmpL1;

wire ds_out;
wire tw_out;
wire tw_next_out;
wire falling0, falling1;

deltasigma #(.BITS(DSBITS)) DS(.clk, .rst, .data_in(ds_fraction), .next(tw_next_out), .out(ds_out));
twister TWISTER(.clk, .rst, .d_in(ds_out), .d_out(tw_out), .next_out(tw_next_out), .next(falling0 | falling1));
 
pwmOC #(.WIDTH(WIDTH), .HRBITS(3)) OC0(.clk(clk), .rst(rst), .tb(tb), .pwmD(pwm0D), .falling(falling0),
	.cmpH('0),
	.cmpL(cmpA + tw_out));

pwmOC #(.WIDTH(WIDTH), .HRBITS(3)) OC1(.clk(clk), .rst(rst), .tb(tb), .pwmD(pwm1D), .falling(falling1),
	.cmpH( {{HRBITS{1'b0}},midPrd}<<HRBITS),
	.cmpL(cmpL1 + tw_out));
	
assign tb_dbg = tb[5];

always_comb begin 
	midPrd = (prd>>1) + 'd1;
	cmpL1 = ({midPrd,{HRBITS{1'b0}}})+cmpA[WIDTH-1:0];
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