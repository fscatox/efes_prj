/**
 * File          : tb_spi_slave.sv
 * Author        : Fabio Scatozza <s315216@studenti.polito.it>
 * Date          : 03.12.2024
 */

import tb_utils_pkg::*;
import tb_spi_master_pkg::*;

module tb_spi_slave;

timeunit 1ns;
timeprecision 1ns;

localparam time Tclk = 20ns; // 50 MHz clock

localparam int unsigned NTransactions = 100;

localparam int unsigned Nbit = 8;
localparam bit Cpol = 0;
localparam bit Cpha = 0;
typedef SpiMaster#(Nbit, Cpol, Cpha) SpiMaster_t;

bit clk, rst_n;
bit [Nbit-1:0] tx_data;
logic [Nbit-1:0] rx_data;
logic tx_strobe, rx_strobe;

//
// SPI bus, master, slave
//

spi_if spi_bus();

spi_slave #(Nbit, Cpol, Cpha) dut (
  .*,
  .ss_n(spi_bus.ss_n),
  .sclk(spi_bus.sclk),
  .mosi(spi_bus.mosi),
  .miso(spi_bus.miso)
);

SpiMaster_t master;

//
// Inter-object analysis communication helpers
// for implementing a simple transaction checker
//
SpiMaster_t::mbox_t mbox;
int unsigned ntrans;

//
// System model
//

initial begin : sys_clk_p
  clk = '0;
  forever #(Tclk/2) clk <= ~clk;
end

initial begin : sys_logic_p
  logic [Nbit-1:0] tx_data_past;
  SpiMaster_t::pkt_t pkt_slave, pkt_master;

  // Initialize checker
  ntrans = 0;

  // Reset spi controller (async)
  rst_n = '0;
  tx_data = '0;

  @(posedge clk);
  rst_n <= '1;

  fork

    // System generates new data at every clock cycle
    forever begin
      tx_data_past <= tx_data;
      tx_data <= lfsr_range(2**$bits(tx_data)-1);
      @(posedge clk);
    end

    // Data to be transmitted last sampled before the tx_strobe pulse
    forever begin
      @(posedge tx_strobe);
      @(posedge clk);

      ntrans++;
      pkt_slave.tx = tx_data_past;
      log("System", $sformatf("SPI #%0d: tx_data = h%h", ntrans, tx_data_past));
    end

    // Incoming data
    forever begin
      @(posedge rx_strobe);
      @(posedge clk);

      pkt_slave.rx = rx_data;
      log("System", $sformatf("SPI #%0d: rx_data = h%h", ntrans, rx_data));

      // Basic checker
      mbox.get(pkt_master);
      if (pkt_slave.tx != pkt_master.rx || pkt_slave.rx != pkt_master.tx)
        log("Checker", "PACKET MISMATCH!", 1);
      if (ntrans == NTransactions)
        log("Checker", "TEST PASSED!", 1);

    end

  join
end

//
// SPI master model
//

initial begin
  mbox = new();
  master = new(spi_bus.master, mbox);
  master.run();
end

endmodule
