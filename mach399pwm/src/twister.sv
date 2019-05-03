// This swaps d0 and d1 every other cycle, to balance things out
module twister(
input wire clk, rst,

input wire d_in,
input wire next,

output wire d_out,
output reg next_out
);

reg [2:0] state;
reg [2:0] state_next;

reg d0, d1;
reg d0_next, d1_next;

assign d_out = d0;

always_ff @(posedge clk or posedge rst) begin
	if(rst) begin
		state <= '0;
		d0 <= '0;
		d1 <= '0;
	end else begin
		state <= state_next;
		d0 <= d0_next;
		d1 <= d1_next;
	end
end

always_comb begin
	state_next = state;
	d0_next = d0;
	d1_next = d1;
	
	next_out = '0;
	
	case (state)
	3'd0: begin
		if(next) begin
			next_out = '1;
			d0_next = d_in;
			state_next = state+1;
		end
	end
	3'd1: begin
		next_out = '1; 	
		d1_next = d_in;
		state_next = state+1;
	end
	3'd2: begin
		if(next) begin
			d0_next = d1;
			state_next = state+1;
		end
	end
	3'd3: begin
		if(next) begin
			next_out = '1; 	
			d1_next = d_in;
			state_next = state+1;
		end
	end
	3'd4: begin
		next_out = '1;
		d0_next = d_in;
		state_next = state+1;
	end
	3'd5: begin
		if(next) begin
			d0_next = d1;
			state_next = '0;
		end
	end
	endcase
end

endmodule