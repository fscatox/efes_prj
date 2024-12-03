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

  function automatic int unsigned lfsr();
    // any non-zero seed
    static bit [31:0] state = 32'hAE1F_B42C;

    // XNOR Taps: 32,22,2,1
    // https://docs.amd.com/v/u/en-US/xapp052
    state = {state[30:0], ~^{state[31],state[21],state[1],state[0]}};
    log("LFSR", $sformatf("state = %0d", state));

    return state;
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
