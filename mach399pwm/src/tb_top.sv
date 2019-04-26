module tb_top();

reg clk_USB;wire PushBn, uart_rx, uart_tx;

wire spi_sclk, spi_csn, spi_mosi, spi_miso;
wire scl, sda;
wire I2CAlert;
wire pwm0, pwm1;

always begin
	clk_USB = '0;
	#41.7ns;
	clk_USB = '1;
	#41.7ns;
end

top DUT (.*);


endmodule
