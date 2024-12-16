/**
 * File          : spi_slave.sv
 * Author        : Fabio Scatozza <s315216@studenti.polito.it>
 * Date          : 02.12.2024
 * Description   : SPI slave interface implemented in the SPI clock domain, with
 *                 programmable CPOL (clock idles low or high) and CPHA (sampling
 *                 is on the leading or trailing clock edge).
 *                 The data to be transmitted is sampled in the system clock
 *                 domain while the spi slave is not selected: in the cycle when
 *                 the tx_strobe signal pulses high, the communication begins
 *                 and the tx_data is latched internally at the value sampled in
 *                 the previous cycle. As the communication ends and the spi
 *                 slave gets deselected, the received data is latched on the
 *                 rx_data output and becomes valid in the clock cycle when the
 *                 rx_strobe signal pulses high.
 */

module spi_slave #(
  parameter int unsigned NBIT = 8,
  parameter bit CPOL = 0,
  parameter bit CPHA = 0
) (
  input var logic clk,
  input var logic rst_n,
  input var logic [NBIT-1:0] tx_data,
  output logic [NBIT-1:0] rx_data,
  output logic tx_strobe,
  output logic rx_strobe,

  input var logic ss_n,
  input var logic sclk,
  input var logic mosi,
  output tri miso
);

logic [NBIT-1:0] tx_q;
logic load_strobe;
logic [3:1] ssn_q; // 2FF+FF ss_n synchronizer and edge detector

logic [NBIT-1:0] shreg_q;
logic rx_bit;
logic sample_clk;
logic gen_clk;
logic shreg_loaded;

//
// Domain clk
//

// ss_n synchronizer to mitigate metastability when crossing clock domains
always_ff @(posedge clk, negedge rst_n) begin
  if (!rst_n)
    ssn_q <= '1;
  else
    ssn_q <= {ssn_q[2:1], ss_n};
end

// strobe signals generated from ss_n edges
assign tx_strobe = ~ssn_q[2] & ssn_q[3];
assign load_strobe = ssn_q[2] & ~ssn_q[3];

// one cycle delay due to rx_data being sampled at the load_strobe
always_ff @(posedge clk, negedge rst_n) begin
  if (!rst_n)
    rx_strobe <= '0;
  else
    rx_strobe <= load_strobe;
end

// data to be transmitted sampled while the slave is not selected
always_ff @(posedge clk)
  if (ssn_q[2])
    tx_q <= tx_data;

// received data sampled at the load strobe
// when CPHA == 1, there's no gen_clk event for shifting the LSB into shreg
always_ff @(posedge clk, negedge rst_n)
  if (!rst_n)
    rx_data <= '0;
  else if (load_strobe)
    rx_data <= ~CPHA ? shreg_q : {shreg_q[NBIT-2:0], rx_bit};

//
// Domain sclk
//

// registers trigger based on CPOL and CPHA
//  CPOL  CPHA  SAMPLE  GEN
//  0     0     pos     neg
//  0     1     neg     pos
//  1     0     neg     pos
//  1     1     pos     neg

assign sample_clk = sclk^(CPOL^CPHA);
assign gen_clk = sclk^~(CPOL^CPHA);

always_ff @(posedge sample_clk)
  rx_bit <= mosi;

// shift/load operation controller
// asynchronous synchronized clr when the slave is not selected for preselecting
// the load operation of the shreg. At the first gen_clk edge and for the
// remaining duration of the communication, the shreg is selected for shifting
always_ff @(posedge gen_clk, posedge ssn_q[2])
  if (ssn_q[2])
    shreg_loaded <= '0;
  else
    shreg_loaded <= '1;

// left shift register, as bits are transmitted MSB first
// when CPHA == 0, the MSB is taken from the txreg instead of being loaded and
// shifted out of shreg
always_ff @(posedge gen_clk)
  if (!shreg_loaded)
    shreg_q <= CPHA ? tx_q : {tx_q[NBIT-2:0], rx_bit};
  else
    shreg_q <= {shreg_q[NBIT-2:0], rx_bit};

// the miso line is shared, with ss_n for preventing conflicts
assign miso = ss_n ? 'z : shreg_loaded ? shreg_q[NBIT-1] : tx_q[NBIT-1];

endmodule
