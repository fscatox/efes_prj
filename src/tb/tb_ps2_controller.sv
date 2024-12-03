/**
 * File          : tb_ps2_controller.sv
 * Author        : Fabio Scatozza <s315216@studenti.polito.it>
 * Date          : 29.11.2024
 */


import tb_utils_pkg::*;
import tb_ps2_device_pkg::*;

module tb_ps2_controller;

timeunit 1ns;
timeprecision 1ns;

const time tclk = 20ns; // 50 MHz clock
const time dt_clk = lfsr_range(tclk-1); // host/device clocks shift

logic clk, rst_n, en;
logic tx_rqst, valid;
logic [7:0] tx_data, rx_data;
ps2_pkg::flags_t flags;

//
// PS2 bus, controller, device
//

ps2_if ps2_bus();

ps2_controller dut (
  .*,
  .ps2_clk(ps2_bus.clk),
  .ps2_dat(ps2_bus.dat)
);

Ps2Device dev;

//
// Host model
//

initial begin : host_clk_p
  clk = 0;
  forever #(tclk/2) clk <= ~clk;
end

initial begin : host_logic_p
  rst_n = '1;
  en = '0;
  tx_rqst = '0;
  tx_data = '0;
  #5;

  // Reset the controller (asynch)
  rst_n <= '0;
  #(tclk/2);

  // Keep the bus inhibited for some clock cycles
  rst_n <= '1;
  repeat (2)
    @(posedge clk);

  forever begin

    // Receive 10 characters
    repeat(10) begin
      en <= '1;
      @(posedge valid);
      @(posedge clk);
      if (~|flags)
        log("Host", $sformatf("rx_data = '0b%b'", rx_data));
      else
        log("Host", $sformatf("Rx failed: %s", flagsToStr(flags)), 1);
    end

    // While receiving another character
    repeat (5)
      @(posedge clk);

    // Request transmission
    tx_data <= lfsr_range(2**$size(tx_data)-1);
    tx_rqst <= '1;

    // By deasserting the request, a valid pulse will be generated
    @(posedge clk);
    tx_rqst <= '0;
    log("Host", $sformatf("Request-to-send: tx_data = '0b%b'", tx_data));

    // tx_data can change, but must have been already latched
    tx_data <= '0;

    @(posedge valid);
    @(posedge clk);
    if (~|flags)
      log("Host", "Tx done");
    else
      log("Host", $sformatf("Tx failed: %s", flagsToStr(flags)), 1);

    // Request a new transmission
    tx_data <= lfsr_range(2**$size(tx_data)-1);
    tx_rqst <= '1;

    // With a 'blocking' valid
    @(posedge clk);
    log("Host", $sformatf("Request-to-send: tx_data = '0b%b'", tx_data));

    @(posedge valid);
    @(posedge clk);
    if (~|flags)
      log("Host", "Tx done");
    else
      log("Host", $sformatf("Tx failed: %s", flagsToStr(flags)), 1);

    // After some time, unblock and repeat
    # 1ms;
    tx_rqst <= '0;
  end
end : host_logic_p

initial begin
  dev = new(ps2_bus.dev, dt_clk);
  dev.run(Ps2Device::NOERR);
end

endmodule
