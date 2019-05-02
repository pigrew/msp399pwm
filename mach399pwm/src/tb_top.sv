`timescale 1 ns / 1 ps

module tb_top();

parameter       clk_cycle   = 2us; 
   
reg clk_USB;wire PushBn, uart_rx, uart_tx;

wire spi_sclk, spi_csn, spi_mosi, spi_miso;
wire scl, sda;
wire I2CAlert;
wire pwm0, pwm1;
wire clk_out;
wire rst_out, d0_out;
wire clkEn; // active low clock enable?

always begin
	clk_USB = 1'b0 & clkEn;
	#41.7ns;
	clk_USB = 1'b1 & clkEn;
	#41.7ns;
end

PUR PUR_INST(.PUR(1'b1));
GSR GSR_INST(.GSR(1'b1));

top DUT (.*);
// 1 MHz fast-mode-plus?

reg scl_reg='z, sda_reg='z;
assign scl = scl_reg;
assign sda = sda_reg;
task write_i2c_start();
	int j;
    #1us;
    sda_reg <= 1'b0;
    #260ns; // t_HD;STA
    scl_reg <= 1'b0;
	#(clk_cycle/4);
    #2 $display($time,": I2C start");
endtask

task write_i2c_data(input   [7:0]   data);
	int j;
    for (j = 7; j >= 0; j = j - 1) begin     // data
		sda_reg = data[j] ? 1'bz : 1'b0;
		#(clk_cycle/4);                          // middle of clock
		scl_reg = 1'bz;
		#(clk_cycle/2);                          // middle of clock
		scl_reg = 1'b0;
		#(clk_cycle/4);                          // middle of clock
    end 
    sda_reg = 1'bz;
	// ACK?
	#(clk_cycle/4);
    scl_reg = 1'bz;
	#(clk_cycle/4);
	if(sda)
		@(negedge sda);
	#(clk_cycle/4);
    scl_reg = 1'b0;
	#(clk_cycle/4);
	
    #2 $display($time,": Addr Write = %h",data);
endtask

task write_i2c_stop();
    sda_reg = 1'b0;
	#(clk_cycle/4);
    scl_reg = 1'bz;
	#(clk_cycle/4);
    sda_reg = 1'bz;
	
    #2 $display($time,": I2C Stop");
endtask
//bit [18:0] cmpa_target = {15'h134,3'h1,1'b0};
bit [18:0] cmpa_target = {15'h134,3'h2,1'b0};
initial begin
	#10us;
	write_i2c_start();
	write_i2c_data(8'b10100100);
	write_i2c_data(8'h00);
	write_i2c_data(cmpa_target[7:0]);
	write_i2c_stop();
	#10us;
	write_i2c_start();
	write_i2c_data(8'b10100100);
	write_i2c_data(8'h01);
	write_i2c_data(cmpa_target[15:8]);
	write_i2c_stop();
	#10us;
	write_i2c_start();
	write_i2c_data(8'b10100100);
	write_i2c_data(8'h02);
	write_i2c_data({6'h00,cmpa_target[17:16]});
	write_i2c_stop();
	#10us;
	write_i2c_start();
	write_i2c_data(8'b10100100);
	write_i2c_data(8'h03);
	write_i2c_data(8'h0);
	write_i2c_stop();
end
endmodule
