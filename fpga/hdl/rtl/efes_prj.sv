/**
 * File          : efes_prj.sv
 * Author        : Fabio Scatozza <s315216@studenti.polito.it>
 * Date          : 14.12.2024
 */

module efes_prj (
    input var logic clk,
    input var logic async_rst_n,

    // SPI port
    input var logic sclk,
    input var logic mosi,
    input var logic ps2_ss_n,
    input var logic adc_ss_n,
    output tri ps2_miso,
    output tri adc_miso_forwarded,

    // PS/2 port
    inout tri ps2_clk,
    inout tri ps2_dat,

    // 7-Seg display peripheral
    input var logic sseg_urx,
    output logic [6:0] sseg_out[6],

    // On-board ADC port
    output logic adc_sclk,
    output logic adc_mosi,
    output logic adc_ss_n_forwarded,
    input  tri   adc_miso
);

  logic rst_n;
  logic ps2_ss_nx, adc_ss_nx;

  // Reset synchronizer
  arst_sync #(
      .SYNC_STAGES(4)
  ) arst_sync_i (
      .clk,
      .async_rst_n,
      .rst_n
  );

  // Exclusively active ss_n
  assign ps2_ss_nx = ps2_ss_n | ~adc_ss_n;
  assign adc_ss_nx = adc_ss_n | ~ps2_ss_n;

  // Expose ADC interface onto GPIO
  assign adc_sclk = sclk;
  assign adc_mosi = mosi;
  assign adc_ss_n_forwarded = adc_ss_nx;
  assign adc_miso_forwarded = adc_ss_nx ? 'z : adc_miso;

  // PS/2 peripheral
  ps2_peripheral_spi_wrapper #(
      .CPOL(0),
      .CPHA(0),
      .FIFO_ADDR_WIDTH(8)
  ) ps2_peripheral_i (
      .clk,
      .rst_n,
      .ss_n(ps2_ss_nx),
      .sclk,
      .mosi,
      .miso(ps2_miso),
      .ps2_clk,
      .ps2_dat
  );

  // 7-Seg display peripheral
  uart_sseg_peripheral #(
      .NDISP(6),
      .HIGH_FIRST(1),
      .SEG_ON(0),
      .PARITY(1),
      .PARITY_TYPE(0),
      .NSTOP(1),
      .FCLK_HZ(50e6),
      .FUART_HZ(115200),
      .TIM_PSC(30)
  ) sseg_peripheral_i (
      .clk,
      .rst_n,
      .urx (sseg_urx),
      .sseg(sseg_out)
  );

endmodule

