module i2cregif
#(
parameter REGBITS = 2

)
(
input wire rst,
input wire clk,

input wire scl_in, output wire scl_oe,
input wire sda_in, output wire sda_oe,

output reg [REGBITS-1:0] regAddr,
output reg [7:0] regData,
output reg regDataValid
);
// This is a write-only i2c controller (no ability to read registers
wire [7:0] data_in = 8'h0;
wire [7:0] data_out;
wire ready = 1'b1; // Assume we're fast enough to be always ready

wire i2c_start, i2c_stop, i2c_data_vld;
wire i2c_start_s, i2c_stop_s, i2c_data_vld_s;
sync2 SYNC_START(.clk, .d(i2c_start), .q(i2c_start_s));
sync2 SYNC_STOP(.clk, .d(i2c_stop), .q(i2c_stop_s));
sync2 SYNC_VLD(.clk, .d(i2c_data_vld), .q(i2c_data_vld_s));

i2c_slave #(.I2C_SLAVE_ADDR(7'b1010010)) i2cSlave_impl (.XRESET(rst), .start(i2c_start), .stop(i2c_stop), .data_vld(i2c_data_vld), .r_w(), .* );

reg [2:0] state, state_next;
reg [REGBITS-1:0] regAddr_next;
reg [7:0] regData_next;
reg regDataValid_next;

always_ff @(posedge clk, posedge rst) begin
	if(rst) begin
		state <= '0;
		regAddr <= '0;
		regData <= '0;
		regDataValid <= '0;
	end else begin
		state <= state_next;
		regAddr <= regAddr_next;
		regData <= regData_next;
		regDataValid <= regDataValid_next;
	end
end

always_comb begin
	state_next = state;
	regAddr_next = regAddr;
	regData_next = '0;
	regDataValid_next = '0;
	if(i2c_start_s) begin // reset
		state_next = 3'd0;
	end else begin
		case (state)
			3'd0: begin
				if (i2c_data_vld_s)
					state_next = 3'd1;
			end
			3'd1: begin
				if (~i2c_data_vld_s) begin
					state_next = 3'd2;
					regAddr_next = data_out[REGBITS-1:0];
				end
			end
			
			3'd2: begin
				if (i2c_data_vld_s)
					state_next = 3'd3;
			end
			3'd3: begin
				if (~i2c_data_vld_s) begin
					state_next = 3'd2;
					regData_next = data_out;
					regDataValid_next = '1;
				end
			end
			3'd4: begin
			end
		endcase
	end
end

endmodule