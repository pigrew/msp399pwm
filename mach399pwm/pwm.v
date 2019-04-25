module pwm
(
input wire clk,
input wire rst,
output wire pwm0I,
output wire pwm0Q,
output wire pwm1I,
output wire pwm1Q,

// Wishbone
input wire wb_clk_i,
input wire wb_rst_i,
input wire [1:0] wb_adr_i,
input wire [7:0] wb_dat_i,
input wire wb_we_i

);
localparam WIDTH=18;
reg [WIDTH-1:0] tb;
reg [WIDTH-1:0] tb_next;

reg [WIDTH-1:0] prd, prd_next;  // PRD is this number, plus 1
wire [18:0] cmpH0;
assign cmpH0 = 18'h0;
wire [17:0] cmpL0;assign CMPL0 = 17'h5;
pwmOC OC0(.clk(clk), .rst(rst), .pwmI(pwm0I), .pwmQ(pwm0Q), .cmpH(chpH0), .cmpL(cmpL0));

wire [18:0] cmpH1;
assign cmpH1 = 18'h4;
wire [17:0] cmpL1;
assign cmpL1 = 17'h3;

pwmOC OC1(.clk(clk), .rst(rst), .pwmI(pwm1I), .pwmQ(pwm1Q), .cmpH(cmpH1), .cmpL(cmpL1));

always @(posedge clk, posedge rst) begin
	if(rst) begin
		prd <= 'h00008;
		tb <= 1'b0;
	end else begin
		tb <= tb_next;
		prd <= prd_next;
	end
end

always @(*) begin
	tb_next = tb-1;
	prd_next = prd;
	if(tb==0) begin
		tb_next = prd;
	end
end

endmodule