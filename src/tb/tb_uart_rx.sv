/**
 * File          : tb_uart_rx.sv
 * Author        : Fabio Scatozza <s315216@studenti.polito.it>
 * Date          : 13.12.2024
 */

import tb_utils_pkg::*;
import tb_uart_tx_pkg::*;

module tb_uart_rx;

timeunit 1ns;
timeprecision 1ns;

localparam int unsigned NTransactions = 1000;

localparam real Fclk = 50e6;
localparam real FclkUart = 115200;
localparam int unsigned UartRxPsc = 30;
localparam time Tclk = 1e9/Fclk;

localparam int unsigned Nchar = 8;
localparam bit Parity = 1;
localparam bit ParityType = 0;
localparam int unsigned Nstop = 1;

typedef UartFrame #(Nchar, Parity, ParityType, Nstop) UartFrame_t;
typedef UartTx #(UartFrame_t, FclkUart) UartTx_t;

bit clk, rst_n;
logic [Nchar-1:0] rx_data;
logic valid;
logic frame_error, parity_error;

//
// UART bus, tx, rx
//

uart_if uart_bus();

uart_rx #(Nchar, Parity, ParityType, Nstop, Fclk, FclkUart, UartRxPsc) dut (
  .*,
  .uart_rx(uart_bus.to_rx)
);

UartTx_t uart_tx;

//
// Inter-object analysis communication helpers
// for implementing a simple transaction checker
//
UartTx_t::mbox_t mbox;
int unsigned ntrans;

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

  // Reset controller
  rst_n = 0;

  @(negedge clk);
  rst_n <= 1;

  while (ntrans < NTransactions) begin
    UartTx_t::apkt_t apkt;

    // wait for a transmission to complete
    // outputs are latched and safe to read on negedge
    @(negedge valid);
    ntrans++;

    // Retrieve the original transaction
    mbox.get(apkt);

    // Check frame error is detected
    if (apkt.frame_error != frame_error)
      log("Checker", "Frame error MISMATCH!", 1);
    if (apkt.parity_error != parity_error)
      log("Checker", "Parity error MISMATCH!", 1);
    if (!apkt.frame_error & (apkt.uframe.char != rx_data))
      log("Checker", "Character MISMATCH!", 1);
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

