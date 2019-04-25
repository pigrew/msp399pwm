module pwm
#(
parameter WIDTH = 18
)
(
input wire clk,
input wire rst,
output wire pwm0I,
output wire pwm0Q,
output wire pwm1I,
output wire pwm1Q
);
reg [WIDTH-1:0] tb;
reg [WIDTH-1:0] tb_next;

reg [WIDTH-1:0] prd, prd_next;  // PRD is this number, plus 1
pwmOC  #(.WIDTH(WIDTH) )OC0(.clk(clk), .rst(rst), .tb(tb), .pwmI(pwm0I), .pwmQ(pwm0Q), .cmpH('h0), .cmpL('h5));

pwmOC #(.WIDTH(WIDTH) ) OC1(.clk(clk), .rst(rst), .tb(tb), .pwmI(pwm1I), .pwmQ(pwm1Q), .cmpH('h4), .cmpL('h3));

always_ff @(posedge clk, posedge rst) begin
	if(rst) begin
		prd <= 'h00008;
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