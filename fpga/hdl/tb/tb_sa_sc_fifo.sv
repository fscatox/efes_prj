/**
 * File          : tb_sa_sc_fifo.sv
 * Author        : Fabio Scatozza <s315216@studenti.polito.it>
 * Date          : 05.12.2024
 */

import tb_utils_pkg::*;
import tb_fifo_pkg::*;

module tb_sa_sc_fifo;

timeunit 1ns;
timeprecision 1ns;

localparam int unsigned NTransactions = 1_000;

localparam int unsigned DataWidth = 8;
localparam int unsigned AddrWidth = 4;
localparam time Tclk = 50ns;

typedef bit [DataWidth-1:0] data2_t;
typedef logic [DataWidth-1:0] data4_t;

int unsigned ntrans;

bit clk;
bit rst_n;
bit sclr;
bit we, ack;
data2_t d;
data4_t q;
logic empty, full;

//
// DUT and golden model
//

sa_sc_fifo #(DataWidth, AddrWidth) dut (.*);
GoldenFifo #(DataWidth, AddrWidth) fifo;

//
// Clock
//

initial begin
  clk = '0;
  forever #(Tclk/2) clk <= ~clk;
end

//
// Driver
//

initial begin
  fifo = new();
  ntrans = 0;
  rst_n = 0;

  // initialization sequence to skip the show-ahead unknowns
  repeat (2) begin
    @(negedge clk);
    rst_n <= 1;
    d <= lfsr_range(2**$bits(d)-1);
    we <= 1;
    ack <= 0;
    sclr <= 0;
  end
  sclr <= 1;

  forever begin

    // Update the cycle-accurate model
    @(posedge clk);
    fifo.clock(sclr, we, ack, d);

    // Run the checker
    @(negedge clk);

    if ($isunknown({empty,full}) | $isunknown(q) & !fifo.empty)
      log("Checker", "DUT generated undefined values", 1);

    if ($isunknown(fifo.q) & !fifo.empty)
      log("Checker", "Golden model generated undefined values while !empty", 1);

    if (fifo.empty != empty | fifo.full != full | fifo.q != q)
      log("Checker", {"MISMATCH\n",
        $sformatf("golden: empty = %b, full = %b, q = h%h\n", fifo.empty, fifo.full, fifo.q),
        $sformatf("dut   : empty = %b, full = %b, q = h%h\n", empty, full, q)}, 1);

    if (++ntrans == NTransactions)
      log("Checker", "TEST PASSED!", 1);

    // Generate new inputs
    d = lfsr_range(2**$bits(d)-1);
    we = fifo.full ? 0 : (lfsr_range(99) < 44 ? 1 : 0);
    ack = fifo.empty ? 0 : (lfsr_range(99) < 14 ? 1 : 0);
    sclr = lfsr_range(99) < 4 ? 1 : 0;

  end
end

endmodule
