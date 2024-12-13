/**
 * File          : tb_uart_sseg_peripheral.sv
 * Author        : Fabio Scatozza <s315216@studenti.polito.it>
 * Date          : 13.12.2024
 */

import tb_utils_pkg::*;
import tb_uart_tx_pkg::*;

module tb_uart_sseg_peripheral;

timeunit 1ns;
timeprecision 1ns;

localparam int NTransactions = 500;

localparam int unsigned Ndisp = 6;
localparam bit HighFirst = 1;
localparam bit SegOn = 0;

localparam real Fclk = 50e6;
localparam real FclkUart = 115200;
localparam int unsigned UartRxPsc = 30;
localparam time Tclk = 1e9/Fclk;

localparam int unsigned Nchar = 8;
localparam bit Parity = 1;
localparam bit ParityType = 0;
localparam int unsigned Nstop = 1;

//
// Custom UART frame with better suited probabilities for the transmitted char
//

class UartSsegFrame
  extends UartFrame #(Nchar, Parity, ParityType, Nstop);

  // replaces ::randomize() for svverifcation license limitations
  virtual function void lfsr_randomize();
    char = lfsr_range(2**NCHAR-1);
    char[7] = lfsr_range(99) < 10 ? 1 : 0; // reset probability
    pbit = ^{PARITY_TYPE, char};
    stop = '1;
  endfunction

endclass

typedef UartTx #(UartSsegFrame, FclkUart) UartTx_t;

bit clk, rst_n;
logic [6:0] sseg[Ndisp];

//
// UART bus, tx, sseg_peripheral
//

uart_if uart_bus();

uart_sseg_peripheral #(
  Ndisp, HighFirst, SegOn,
  Parity, ParityType, Nstop,
  Fclk, FclkUart, UartRxPsc
) dut (
  .*,
  .urx(uart_bus.to_rx)
);

UartTx_t uart_tx;

//
// Inter-object analysis communication helpers
// for implementing a simple transaction checker
//

UartTx_t::mbox_t mbox;
int unsigned ntrans;

// Golden model helpers
typedef bit [6:0] sseg_golden_t [Ndisp];

sseg_golden_t sseg_golden;
int unsigned sseg_ptr;

function automatic void sseg_golden_reset();
  sseg_ptr = HighFirst ? Ndisp-1 : 0;
  sseg_golden = '{default: ~{7{SegOn}}};
endfunction

function automatic void sseg_ptr_next();
  if (HighFirst)
    sseg_ptr = !sseg_ptr ? Ndisp-1 : sseg_ptr-1;
  else
    sseg_ptr = (sseg_ptr+1) % Ndisp;
endfunction

//
// System model
//

initial begin : sys_clk_p
  clk = 0;
  forever #(Tclk/2) clk <= ~clk;
end

initial begin : sys_logic_p
  // Initialize checker
  ntrans = 0;

  // Reset peripheral
  rst_n = 0;
  sseg_golden_reset();

  @(negedge clk);
  rst_n <= 1;

  // Check reset behavior
  if ($isunknown(sseg) | sseg_golden_t'(sseg) != sseg_golden)
      log("Checker", "Failed to clear ssegs (rst_n)", 1);

  while (ntrans < NTransactions) begin
    UartTx_t::apkt_t apkt;

    // wait for the transmission to end (looking into the dut)
    @(negedge dut.valid);

    // wait for the update to occur
    @(negedge clk);

    // get the applied packet
    mbox.get(apkt);
    ntrans++;

    // update golden copy
    if (~(apkt.frame_error|apkt.parity_error)) begin
      if (apkt.uframe.char[7])
        sseg_golden_reset();
      else begin
        sseg_golden[sseg_ptr] = apkt.uframe.char[6:0];
        sseg_ptr_next();
      end
    end

    // Check for equivalence
    if ($isunknown(sseg) | sseg_golden_t'(sseg) != sseg_golden)
      log("Checker", "MISMATCH!", 1);
  end

  log("Checker", "TEST PASSED!", 1);

end

//
// UartTx model
//

initial begin
  mbox = new();
  uart_tx = new(uart_bus, mbox);
  uart_tx.run(.inject_errors(1));
end

endmodule

