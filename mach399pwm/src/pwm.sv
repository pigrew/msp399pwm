module pwm
#(
parameter WIDTH = 18,
parameter HRBITS = 3
)
(
input wire clk,
input wire rst,
output wire [(1<<HRBITS)-1:0] pwm0D,
output wire [(1<<HRBITS)-1:0] pwm1D
);

reg [WIDTH-HRBITS-1:0] tb;
reg [WIDTH-HRBITS-1:0] tb_next;

reg [WIDTH-HRBITS-1:0] prd, prd_next;  // PRD is this number, plus 1

pwmOC #(.WIDTH(WIDTH), .HRBITS(3)) OC0(.clk(clk), .rst(rst), .tb(tb), .pwmD(pwm0D), .cmpH('h0), .cmpL('h2D709));

pwmOC #(.WIDTH(WIDTH), .HRBITS(3)) OC1(.clk(clk), .rst(rst), .tb(tb), .pwmD(pwm1D), .cmpH('h20000), .cmpL('hD709));

always_ff @(posedge clk, posedge rst) begin
	if(rst) begin
		prd <= 'h3ffff;
		tb <= 1'b0;
	end else begin
		tb <= tb_next;
		prd <= prd_next;
	end
end

always_comb begin
	tb_next = tb-1;
	prd_next = prd;
	if(tb==0) begin
		tb_next = prd;
	end
end

endmodule