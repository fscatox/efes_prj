/**
 * File          : tb_ps2_controller_errors.sv
 * Author        : Fabio Scatozza <s315216@studenti.polito.it>
 * Date          : 29.11.2024
 */

import tb_utils_pkg::*;
import tb_ps2_device_pkg::*;

module tb_ps2_controller_errors;

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
Ps2Device::err_t err;

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

  // FERR_START, ..., RX_CLK_TO_0, RQST_TO, ..., TX_CLK_TO_9
  repeat(25) begin
    en <= '1;

    if (err >= Ps2Device::RQST_TO) begin
      tx_rqst <= '1;
      tx_data <= lfsr_range(2**$size(tx_data)-1);
      @(posedge clk);
      tx_rqst <= '0;
    end

    @(posedge valid);
    @(posedge clk);

    assert(|flags)
      log("Host", $sformatf("Got error: %s (%s)", flagsToStr(flags), err.name()));
    else
      log("Host", $sformatf("Error uncaught (%s)", err.name()), 1);

    // Inhibit the bus and restart the controller
    en <= '0;
    repeat (150us/tclk)
      @(posedge clk);

    // local count
    err = err.next();
  end

  log("Checker", "TEST PASSED!", 1);

end : host_logic_p

initial begin
  dev = new(ps2_bus.dev, dt_clk);
  err = Ps2Device::FERR_START; // cycle through all errors
  dev.run(err);
end

endmodule
