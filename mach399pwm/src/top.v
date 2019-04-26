module top (
	input wire clk_USB,
	input wire PushBn,
	// UART
	input   uart_rx,
	output  uart_tx,

	// SPI
	output  wire spi_sclk, spi_csn, spi_mosi,
	input   wire spi_miso,
	
	// I2C
	inout   scl,
	inout   sda,
	output  I2CAlert, //TEMP SIGNAL

	
	output wire pwm0, pwm1
);
wire pllLock;
wire clkb;

pllImpl __ (.CLKI(clk_USB), .RST(~PushBn), .CLKOP(clkb ), .LOCK(pllLock));

assign scl = 1'b1;
assign sda = 1'b1;
assign spi_csn = 1'b1;
assign spi_sclk = 1'b1;
assign I2CAlert=1'b1;
assign spi_mosi = spi_miso;
assign uart_tx = uart_rx;
wire pwm0I, pwm0Q, pwm1I, pwm1Q;
wire rst;assign rst = ~pllLock;
pwm pwmA( .clk(clkb), .rst(rst), .pwm0I(pwm0I), .pwm0Q(pwm0Q), .pwm1I(pwm1I), .pwm1Q(pwm1Q) );

ODDRXE opwm0 (.SCLK(clkb), .RST(rst), .D0(pwm0I), .D1(pwm0Q), .Q(pwm0));
ODDRXE opwm1 (.SCLK(clkb), .RST(rst), .D0(pwm1I), .D1(pwm1Q), .Q(pwm1));
endmodule