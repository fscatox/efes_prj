/**
 * File          : ps2_peripheral_spi_wrapper.sv
 * Author        : Fabio Scatozza <s315216@studenti.polito.it>
 * Date          : 05.12.2024
 * Description   : SPI wrapper for the PS/2 controller module.
 *
 *                 The exchanged packets have the following format:
 *
 *                  MISO (sent @ spi_slave.tx_strobe)
 *                  15                             8        |<--- FLAGS --->|     0
 *                   ---------------------------------------------------------------
 *                  |                               |   |   |   |   |   |   |   |   |
 *                  |              DATA             |RXV|TXC|RTO|CTO| PE| FE| OE| EN|
 *                  |                               |   |   |   |   |   |   |   |   |
 *                   ---------------------------------------------------------------
 *
 *                  MOSI (received @ spi_slave.rx_strobe)
 *                   ---------------------------------------------------------------
 *                  |                               |   |   |   |   |   |   |   |   |
 *                  |              DATA             |TX |   |   |   |   |BC |CEN|WEN|
 *                  |                               |EN |   |   |   |   |LR |   |   |
 *                   ---------------------------------------------------------------
 *
 *                  The SPI communication is full-duplex, but the data sent by the master is
 *                  disregarded unless the WEN bit is set. If the WEN bit set:
 *
 *                    * CEN (Controller Enable)
 *                      This bit is set to enable the PS/2 controller and clear the output flags:
 *                      FE, PE, CTO, and RTO. Clearing the CEN bit disables the PS/2 controller and
 *                      inhibits the PS/2 bus (clock line forced low). If the controller is already
 *                      running (EN bit is set) and the CEN bit is set, the current operation is
 *                      aborted and the controller is restarted in the operation mode given by TXEN.
 *
 *                      If the PS/2 controller gets disabled while the PS/2 device is transmitting
 *                      data, the PS/2 device may take up to 100us to abort. The PS/2 controller
 *                      does not prevent being enabled during this interval, which may result in
 *                      a failed data reception (FE, PE, or CTO set).
 *
 *                    * BCLR (Buffer Clear)
 *                      This bit is set to flush the RX FIFO buffer. If both CEN and BCLR are set,
 *                      the flush operation is performed prior to enabling the controller.
 *
 *                    * TXEN (Transmitter Mode Enable)
 *                      This bit determines the operation mode.
 *
 *                      When cleared, the PS/2 controller receives data from the PS/2 device. The
 *                      incoming packets are stored in the RX FIFO buffer, whose state is encoded in
 *                      the RXV and OE bits. With an empty FIFO the RXV bit reads 0 and the DATA
 *                      field is undefined. Otherwise, RXV is set marking the validity of the DATA
 *                      field. If the FIFO buffer is full and new packets are received, the OE bit
 *                      is set. Each SPI transfer removes one element from the FIFO, possibly
 *                      clearing the OE bit. Alternatively, the OE bit is cleared by flushing the
 *                      FIFO buffer with the BCLR bit.
 *
 *                      When set, the FIFO buffer is cleared and the PS/2 controller sends data to
 *                      the PS/2 device. When the operation completes, the TXC bit is set and the
 *                      PS/2 bus is inhibited (EN bit is cleared). If any error has occurred, the
 *                      corresponding flag is set as well.
 *
 *                  As for the data received by the master:
 *
 *                    * EN (Controller Enabled)
 *                      This bit is set by the master when enabling the PS/2 controller and cleared
 *                      by the hardware in one of the following cases:
 *                        - the PS/2 communication fails and a flag is raised (FE, PE, CTO, or RTO)
 *                        - the TXEN bit enabled the PS/2 controller in transmitter mode and the
 *                        transmission has terminated.
 *                      With the EN bit cleared, the PS/2 bus is inhibited (clock line forced low).
 *
 *                    * OE (Overrun Error)
 *                      This bit is set by the hardware when the FIFO RX buffer is full and
 *                      a new packet has been lost. It is automatically cleared when removing
 *                      elements from the FIFO with SPI transfers, or by flushing the buffer with
 *                      the BCLR bit.
 *
 *                    * FE (Frame Error)
 *                      This bit is set when the PS/2 communication fails in one of the following
 *                      ways. While receiving data from the PS/2 device, if either the START or
 *                      STOP bits are missing. While transmitting data to the PS/2 device, if the
 *                      ACK bit is missing. Once the error is raised, the PS/2 controller stops and
 *                      inhibits the PS/2 bus (clock line forced low). Normal operation is resumed
 *                      by enabling the controller.
 *
 *                    * PE (Parity Error)
 *                      This bit is set when the PS/2 device sends data and the parity bit is wrong.
 *                      Once the error is raised, the PS/2 controller stops and inhibits the PS/2
 *                      bus (clock line force low). Normal operation is resumed by enabling the
 *                      controller.
 *
 *                    * CTO (Clock Timeout)
 *                      This bit is set when the PS/2 communication fails because of early clock
 *                      termination, either while sending or receiving data from the PS/2 device.
 *                      Once the error is raised, the PS/2 controller stops and inhibits the PS/2
 *                      bus (clock line forced low). Normal operation is resumed by enabling the
 *                      controller.
 *
 *                    * RTO (Request Timeout)
 *                      This bit is set when the PS/2 controller is about to send data to the PS/2
 *                      device, and the device fails to generate the clock within a 15 ms window
 *                      following the falling edge that started the request-to-send bus
 *                      configuration. Once the error is raised, the PS/2 controller stops and
 *                      inhibits the PS/2 bus (clock line forced low). Normal operation is resumed
 *                      by enabling the controller.
 *
 *                    * TXC (Transmission Complete)
 *                      This bit is set when the PS/2 controller is enabled in transmitter mode and
 *                      the transmission has completed (with or without errors).
 *
 *                    * RXV (Receive Data Valid)
 *                      This bit is set when the byte in the DATA field is valid and contains one
 *                      packet received over PS/2 from the device. The element is automatically
 *                      removed from the RX FIFO buffer, making space for new data.
 */

module ps2_peripheral_spi_wrapper #(
  parameter bit CPOL = 0,
  parameter bit CPHA = 0,
  parameter int unsigned FIFO_ADDR_WIDTH = 8
) (
  input var logic clk,
  input var logic rst_n,

  // SPI system port
  input var logic ss_n,
  input var logic sclk,
  input var logic mosi,
  output tri miso,

  // PS/2 device port
  inout tri ps2_clk,
  inout tri ps2_dat
);

ps2_pkg::miso_pkt_t miso_pkt;
ps2_pkg::mosi_pkt_t mosi_pkt;
logic tx_strobe;
logic rx_strobe;
logic wr_strobe;

logic en, en_next;
logic tx_rqst;
logic valid;
ps2_pkg::flags_t flags;
logic ps2_rxc;
logic error;

logic [7:0] ps2_rx_data;
logic sclr;
logic we;
logic ack;
logic empty;
logic full;
logic fifo_wr;

// Pulses when the SPI transfer completes and the master has written data
assign wr_strobe = rx_strobe & mosi_pkt.wen;

// Forces the ps2_controller enable signal low for a cycle, then high/low
// depending on the CEN bit
always_ff @(posedge clk, negedge rst_n)
  if (!rst_n)
    {en_next, en} <= '0;
  else begin
    if (wr_strobe) begin
      en_next <= mosi_pkt.cen;
      en <= '0;
    end
    else
      en <= en_next;
  end

// Loads the TXEN bit, so that once the ps2 transmission has ended, the
// tx_rqst signal is kept high and the PS2 bus is inhibited
always_ff @(posedge clk, negedge rst_n)
  if (!rst_n)
    tx_rqst <= '0;
  else if (wr_strobe)
    tx_rqst <= mosi_pkt.txen;

// Status data of the PS/2 controller
assign ps2_rxc = ~tx_rqst & valid;
assign error = |flags;
assign miso_pkt.txc = tx_rqst & valid;
assign miso_pkt.en = en & ~miso_pkt.txc & ~error;

// {<<{}} not supported
assign miso_pkt.rto = flags.rqst_timeout;
assign miso_pkt.cto = flags.clk_timeout;
assign miso_pkt.pe = flags.parity_error;
assign miso_pkt.fe = flags.frame_error;

// FIFO cleared explicitly or for every PS/2 transmission
assign sclr = tx_rqst | (wr_strobe & mosi_pkt.bclr);

// Not-full FIFO written when the PS/2 controller has successfully received data
assign fifo_wr = ps2_rxc & ~error;
assign we = fifo_wr & ~full;

// Not-empty FIFO pops data out when the SPI transfer begins
assign ack = tx_strobe & ~empty;

// Status data of the FIFO
assign miso_pkt.rxv = ~empty;
always_ff @(posedge clk, negedge rst_n)
  if (!rst_n)
    miso_pkt.oe <= '0;
  else begin
    if (!full)
      miso_pkt.oe <= '0;
    else if (fifo_wr)
      miso_pkt.oe <= '1;
  end

spi_slave #(.NBIT($bits(miso_pkt)), .CPOL(CPOL), .CPHA(CPHA)) spi_slave_i (
  .*,
  .tx_data(miso_pkt),
  .rx_data(mosi_pkt)
);

ps2_controller ps2_controller_i (
  .*,
  .tx_data(mosi_pkt.data),
  .rx_data(ps2_rx_data)
);

sa_sc_fifo #(.DATA_WIDTH($bits(ps2_rx_data)), .ADDR_WIDTH(FIFO_ADDR_WIDTH)) fifo_i (
  .*,
  .d(ps2_rx_data),
  .q(miso_pkt.data)
);

endmodule

