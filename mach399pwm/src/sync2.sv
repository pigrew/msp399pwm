module sync2(
input wire clk,
input wire d,
output reg q
);
reg q1;
always_ff @(posedge clk) begin
	q1 <= d;
	q <= q1;
end
endmodule