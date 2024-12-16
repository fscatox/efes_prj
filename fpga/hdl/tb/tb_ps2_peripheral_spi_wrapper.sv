/**
 * File          : tb_ps2_peripheral_spi_wrapper.sv
 * Author        : Fabio Scatozza <s315216@studenti.polito.it>
 * Date          : 08.12.2024
 */

import tb_utils_pkg::*;
import tb_ps2_device_pkg::*;
import tb_spi_master_pkg::*;
import ps2_pkg::*;

module tb_ps2_peripheral_spi_wrapper;

timeunit 1ns;
timeprecision 1ns;

localparam time Tclk = 20ns; // 50 MHz clock

localparam int unsigned Nbit = 16;
localparam bit Cpol = 0;
localparam bit Cpha = 0;
localparam int unsigned FifoAddrWidth = 4;

bit clk, rst_n;

ps2_if ps2_bus();
spi_if spi_bus();
ps2_peripheral_spi_wrapper #(Cpol, Cpha, FifoAddrWidth) dut (
  .*,

  .ss_n(spi_bus.ss_n),
  .sclk(spi_bus.sclk),
  .mosi(spi_bus.mosi),
  .miso(spi_bus.miso),

  .ps2_clk(ps2_bus.clk),
  .ps2_dat(ps2_bus.dat)
);

Ps2Device ps2_dev;
SpiMaster #(Nbit, Cpol, Cpha) spi_master;

function automatic string misoPktToStr(miso_pkt_t miso_pkt);
  string s = "MISO Packet\n";
  s = {s, $sformatf("data: h%h\n", miso_pkt.data)};
  s = {s, $sformatf("rxv: %b\n", miso_pkt.rxv)};
  s = {s, $sformatf("txc: %b\n", miso_pkt.txc)};
  s = {s, $sformatf("rto: %b\n", miso_pkt.rto)};
  s = {s, $sformatf("cto: %b\n", miso_pkt.cto)};
  s = {s, $sformatf("pe: %b\n", miso_pkt.pe)};
  s = {s, $sformatf("fe: %b\n", miso_pkt.fe)};
  s = {s, $sformatf("oe: %b\n", miso_pkt.oe)};
  s = {s, $sformatf("en: %b\n", miso_pkt.en)};
  return s;
endfunction

//
// System Model
//

initial begin : sys_clk_p
  clk = '0;
  forever #(Tclk/2) clk <= ~clk;
end

initial begin
  ps2_pkg::mosi_pkt_t mosi_pkt;
  ps2_pkg::miso_pkt_t miso_pkt;
  int i;

  // Reset peripheral
  rst_n = '0;
  @(negedge clk);
  rst_n <= '1;

  // 1) While not enabled, get the state
  @(negedge clk);
  mosi_pkt = '0;
  spi_master.transfer(mosi_pkt, miso_pkt);
  assert (!miso_pkt)
    log("Checker", {"Status after reset\n", misoPktToStr(miso_pkt)});
  else
    log("Checker", "Wrong status after reset", 1);

  // Enable the controller in receive mode
  @(negedge clk);
  mosi_pkt.wen = 1;
  mosi_pkt.cen = 1;
  spi_master.transfer(mosi_pkt, miso_pkt);

  // Wait for the FIFO buffer to overflow
  // (can't check the status through SPI otherwise the FIFO would be read)
  repeat (2**FifoAddrWidth * 1.2ms / Tclk)
    @(negedge clk);

  // 2) Check it is full and clear it
  mosi_pkt.wen = 1;
  mosi_pkt.cen = 0;
  mosi_pkt.bclr = 1;
  spi_master.transfer(mosi_pkt, miso_pkt);

  assert (miso_pkt.oe)
    log("Checker", {"FIFO is full\n", misoPktToStr(miso_pkt)});
  else
    log("Checker", "Expected full FIFO", 1);

  // (2 clock cycles for internal processing)
  repeat (3)
    @(negedge clk);

  // (the controller is kept disabled for some PS/2 clock cycles)
  repeat (100us/Tclk)
    @(negedge clk);

  // 3) Check it is empty, then fill it again
  mosi_pkt.wen = 1;
  mosi_pkt.cen = 1;
  mosi_pkt.bclr = 0;
  spi_master.transfer(mosi_pkt, miso_pkt);

  assert (!miso_pkt[7:0])
    log("Checker", {"FIFO cleared\n", misoPktToStr(miso_pkt)});
  else
    log("Checker", "Expected empty FIFO", 1);

  // Wait for the FIFO buffer to overflow
  // (can't check the status through SPI otherwise the FIFO would be read)
  repeat (2**FifoAddrWidth * 1.2ms / Tclk)
    @(negedge clk);

  // 4) Completely empty the FIFO, one SPI transfer at a time
  i = 1;
  mosi_pkt.wen = 1;
  mosi_pkt.cen = 0;
  spi_master.transfer(mosi_pkt, miso_pkt);
  log("System", {$sformatf("Reading FIFO[#%0d] ...\n", i), misoPktToStr(miso_pkt)});

  do begin
    // (2 clock cycles for internal processing)
    repeat (3)
      @(negedge clk);

    mosi_pkt.wen = 0;
    spi_master.transfer(mosi_pkt, miso_pkt);
    i++;
    log("System", {$sformatf("Reading FIFO[#%0d] ...\n", i), misoPktToStr(miso_pkt)});
  end
  while (miso_pkt.rxv & i < 18);

  assert (i == 17 & !miso_pkt[7:0])
    log("Checker", {"FIFO read\n", misoPktToStr(miso_pkt)});
  else
    log("Checker", "FIFO read ended prematurely", 1);

  // (the controller is kept disabled for some PS/2 clock cycles)
  repeat (100us/Tclk)
    @(negedge clk);

  // 5) Start filling the FIFO again
  mosi_pkt.wen = 1;
  mosi_pkt.cen = 1;
  spi_master.transfer(mosi_pkt, miso_pkt);

  repeat (2**(FifoAddrWidth-1) * 1.2ms / Tclk)
    @(negedge clk);

  // 5) Then stop by sending data to the PS/2 device
  mosi_pkt = '{data: lfsr_range(2**8-1), txen: 1, cen: 1, wen: 1, default: 0};
  spi_master.transfer(mosi_pkt, miso_pkt);

  // poll for finish
  do begin

    // (2 clock cycles for internal processing)
    repeat (1us/Tclk)
      @(negedge clk);

    mosi_pkt.wen = 0;
    spi_master.transfer(mosi_pkt, miso_pkt);
    log("System", {"Polling txc\n", misoPktToStr(miso_pkt)});
  end
  while (!miso_pkt.txc);

  assert (~|miso_pkt[5:2])
    log("Checker", {"Tx done\n", misoPktToStr(miso_pkt)});
  else
    log("Checker", "Tx failed", 1);

  $stop;

end

initial begin
  ps2_dev = new(ps2_bus.dev);
  spi_master = new(spi_bus.master);
  ps2_dev.run(Ps2Device::NOERR);
end

endmodule

