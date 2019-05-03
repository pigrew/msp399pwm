module top
#(
parameter PWMWIDTH=19,
parameter DSBITS=8)
(
	input wire clk_USB,
//	input wire PushBn,
	// UART
//	input   uart_rx,
//	output  uart_tx,

	// SPI
//	output  wire spi_sclk, spi_csn, spi_mosi,
//	input   wire spi_miso,
	output rst_out,
	output wire d0_out,
	// I2C Slave?
	inout   scl,
	inout   sda,
//	output  I2CAlert, //TEMP SIGNAL
	
	output wire pwm0, pwm1,
	output wire clkEn,
	output wire [7:0] LED
)
 /* synthesis syn_hier = "flatten" */;
 /************** Clocks ***************/
assign clkEn = 1'b1;
wire pllLock;
wire clk8, clk8s, clk1d;
wire clk_osc;
OSCH #(.NOM_FREQ("19.0")) osc(.STDBY(1'b0), .OSC(clk_osc), .SEDSTDBY());

pllImpl __pll (.CLKI(clk_USB), .RST(1'b0), .CLKOP(clk8),.LOCK(pllLock));
/*assign spi_csn = 1'b1;
assign spi_sclk = 1'b1;
assign I2CAlert=1'b1;
assign spi_mosi = spi_miso;
assign uart_tx = uart_rx;
*/
wire [7:0] pwm0D;
wire [7:0] pwm1D;

wire rst;
assign rst = ~pllLock;

/************** CLK Dividers ************/
ECLKSYNCA eclksa (.ECLKI(clk8), .STOP(1'b0), .ECLKO(clk8s));

CLKDIVC #(.DIV("4.0")) divddr  (.RST(rst), .CLKI(clk8s), .ALIGNWD(1'b0), .CDIV1(), .CDIVX(clk1d) );
assign rst_out = 1'b0;
wire tb_dbg;
assign d0_out = 1'b0;
/************ PWM ****************/
reg [PWMWIDTH-1:0] cmpa, cmpa_scratch;
reg [7:0] dsFrac, dsFrac_scratch;
reg dirty;
assign LED[7:1] ='1;
assign LED[0] = ~dirty;

pwm #(.WIDTH(PWMWIDTH), .DSBITS(DSBITS), .PERIOD(16'hFFFF))pwmA (
	.clk(clk1d), .rst(rst),
	.cmpA(cmpa), .ds_fraction(dsFrac[DSBITS-1:0]), .pwm0D(pwm0D), .pwm1D(pwm1D), .tb_dbg);

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
	
/**************** I2C *****************/
wire scl_oe, scl_in;
wire sda_oe, sda_in;
wire [2:0] regAddr;
wire [7:0] regData;
wire regDataValid;

always @(posedge clk1d, posedge rst) begin
	if(rst) begin
		cmpa <= {16'ha000,3'h5};
		cmpa_scratch <= {16'ha000,3'h5};
		dsFrac <= 8'h12;
	end else begin
		if(regDataValid) begin
			case(regAddr)
				3'd0: begin
					cmpa_scratch[7:0] <= regData;
					dirty <= '1;
				end
				3'd1: begin
					cmpa_scratch[15:8] <= regData;
					dirty <= '1;
				end
				3'd2: begin
					cmpa_scratch[PWMWIDTH-1:16] <= regData;
					dirty <= '1;
				end
				3'd3: begin
					dsFrac_scratch <= regData;
					dirty <= '1;
				end
				3'd4: begin
					cmpa <= cmpa_scratch;
					dsFrac <= dsFrac_scratch;
					dirty <= '0;
				end
			endcase
		end
	end
end

i2cregif #(.REGBITS(3)) i2cregifImpl(
	.clk(clk1d), .rst, .regAddr, .regData, .regDataValid,
	.scl_oe, .scl_in, .sda_oe, .sda_in);
	
BBPU i2c_scl_bbpu (.I(1'b0), .T(~scl_oe), .O(scl_in), .B(scl));
BBPU i2c_sda_bbpu (.I(1'b0), .T(~sda_oe), .O(sda_in), .B(sda));
endmodule