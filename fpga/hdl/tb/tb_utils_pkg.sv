/**
 * File          : tb_utils_pkg.sv
 * Author        : Fabio Scatozza <s315216@studenti.polito.it>
 * Date          : 03.12.2024
 */

package tb_utils_pkg;

  timeunit 1ns;
  timeprecision 1ns;

  //
  // Randomization utilities
  // for toolchains lacking systemverilog verification licenses
  //

  // if made a static variable of lfsr(), not updated
  // (unless for instance, printed with log())
  bit [31:0] lfsr_state = 32'hAE1F_B42C;

  function automatic int unsigned lfsr();
    // XNOR Taps: 32,22,2,1
    // https://docs.amd.com/v/u/en-US/xapp052
    lfsr_state = {lfsr_state[30:0], ~^{lfsr_state[31],lfsr_state[21],lfsr_state[1],lfsr_state[0]}};

    return lfsr_state;
  endfunction

  function automatic int unsigned lfsr_range(int unsigned maxval, int unsigned minval = 0);
    return minval + (lfsr() % (maxval-minval+1));
  endfunction

  //
  // Logging utilities
  //

  function automatic void log(string who, string what, bit stop = 0);
    $display("[%0t] [%s] %s", $time, who, what);
    if (stop)
      $stop;
  endfunction

endpackage
