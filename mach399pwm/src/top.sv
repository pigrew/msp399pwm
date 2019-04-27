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
)
 /* synthesis syn_hier = "flatten" */;
wire pllLock;
wire clk8, clk8s, clk1d;

pllImpl __ (.CLKI(clk_USB), .RST(~PushBn), .CLKOP(clk8),.LOCK(pllLock));

assign scl = 1'b1;
assign sda = 1'b1;
assign spi_csn = 1'b1;
assign spi_sclk = 1'b1;
assign I2CAlert=1'b1;
assign spi_mosi = spi_miso;
assign uart_tx = uart_rx;

wire [7:0] pwm0D;
wire [7:0] pwm1D;

wire rst;assign rst = ~pllLock;

pwm pwmA( .clk(clk1d), .rst(rst), .pwm0D(pwm0D), .pwm1D(pwm1D));

ECLKSYNCA eclksa (.ECLKI(clk8), .STOP(1'b0), .ECLKO(clk8s));

CLKDIVC #(.DIV("4.0")) divddr  (.RST(rst), .CLKI(clk8s), .ALIGNWD(1'b0), .CDIVX(clk1d) );

wire buf_pwm0, buf_pwm1;

OB OB_pwm0 (.I(buf_pwm0), .O(pwm0))
		 /* syn thesis IO _TYPE="LVC MOS33" */;

OB OB_pwm1 (.I(buf_pwm1), .O(pwm1))
		 /* syn thesis IO _TYPE="LVC MOS33" */;
		 
ODDRX4B opwm0 (
	.ECLK(clk8s), .SCLK(clk1d), .RST(rst), .Q(buf_pwm0),
	.D0(pwm0D[0]), .D1(pwm0D[1]), .D2(pwm0D[2]), .D3(pwm0D[3]),
	.D4(pwm0D[4]), .D5(pwm0D[5]), .D6(pwm0D[6]), .D7(pwm0D[7])
	);
ODDRX4B opwm1 (
	.ECLK(clk8s), .SCLK(clk1d), .RST(rst), .Q(buf_pwm1),
	.D0(pwm1D[0]), .D1(pwm1D[1]), .D2(pwm1D[2]), .D3(pwm1D[3]),
	.D4(pwm1D[4]), .D5(pwm1D[5]), .D6(pwm1D[6]), .D7(pwm1D[7])
	);

	//ODDRXE opwm0 (.SCLK(clkb), .RST(rst), .D0(pwm0I), .D1(pwm0Q), .Q(pwm0));
//ODDRXE opwm1 (.SCLK(clkb), .RST(rst), .D0(pwm1I), .D1(pwm1Q), .Q(pwm1));
endmodule