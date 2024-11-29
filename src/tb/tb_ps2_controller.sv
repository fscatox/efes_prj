/**
 * File          : tb_ps2_controller.sv
 * Author        : Fabio Scatozza <s315216@studenti.polito.it>
 * Date          : 29.11.2024
 */

module tb_ps2_controller;

timeunit 1ns;
timeprecision 1ns;

// Clock specifications
time host_tclk = 20ns; // 50 MHz host clock
time dev_tclk = 50us; // 20 kHz device clock
time dt_clks = lfsr_range(host_tclk-1); // dev_tclk is out of phase

// PS2 Timing specifications
time dev_t1 = lfsr_range(25,5)*1us; // from DATA transition to falling edge

// PS/2 open-drain lines
// (net continuously driven at '1 with pull strength)
tri1 ps2_clk, ps2_dat;

// Host interface
logic clk, rst_n, en;
logic tx_rqst, valid;
logic [7:0] tx_data, rx_data;
ps2_pkg::flags_t flags;

// Device model
bit dev_clk;
bit dev_ps2_clk, dev_ps2_dat;

//
// Clocks
//

initial begin
  clk = 0;
  forever #(host_tclk/2) clk <= ~clk;
end

initial begin
  dev_clk = 1;
  #dt_clks;
  forever #(dev_tclk/2) dev_clk <= ~dev_clk;
end

//
// Device behavior
//

// open-drain buffers driven by the device logic
assign ps2_clk = dev_ps2_clk ? 'z : '0;
assign ps2_dat = dev_ps2_dat ? 'z : '0;

// device logic
initial begin
  dev_ps2_dat = '1;
  dev_ps2_clk = '1;

  forever begin

    @(posedge dev_clk);
    $display("[%0t] [DEVICE] Checking the bus state", $time);

    if (ps2_clk)
      // Not inhibited
      if (ps2_dat) begin
        // Not a request-to-send
        devTx();
      end
      else begin
      end
  end
end

//
// Host logic
//
initial begin
  rst_n = '1;
  en = '0;
  tx_rqst = '0;
  #5;

  // Reset the controller (asynch)
  rst_n = '0;
  #(host_tclk/2);

  // Keep the bus inhibited for some clock cycles
  rst_n = '1;
  #(2*host_tclk);

  // Monitor incoming transmissions
  forever begin
    en = 1;
    @(posedge valid);
    @(posedge clk);
    if (~|flags)
      $display("[%0t] [HOST] rx_data = '0b%b'", $time, rx_data);
    else begin
      $display("[%0t] [HOST] %s", $time, flagsToStr(flags));
    end
  end
end

ps2_controller dut (.*);

function automatic string flagsToStr(ps2_pkg::flags_t flags);
  unique case (1'b1)
    flags.frame_error: return "frame error";
    flags.parity_error: return "parity error";
    flags.clk_timeout: return "clock timeout";
    flags.rqst_timeout: return "request-to-send timeout";
  endcase
endfunction

function automatic int unsigned lfsr();
  // any non-zero seed
  static bit [31:0] state = 32'hAE1F_B42C;

  // XNOR Taps: 32,22,2,1
  // https://docs.amd.com/v/u/en-US/xapp052
  state = {state[30:0], ~^{state[31],state[21],state[1],state[0]}};

  // $display("[%0t] [LFSR] %0d", $time, state);
  return state;
endfunction

function automatic int unsigned lfsr_range(int unsigned maxval, int unsigned minval = 0);
  int unsigned ret = minval + (lfsr() % (maxval-minval+1));
  // $display("[%0t] [LFSR_RANGE] %0d <= %0d <= %0d", $time, minval, ret, maxval);
  return ret;
endfunction

// See: IBM PS/2 Reference Manuals, Keyboard/Auxiliary Device Controller, 1990
task automatic devTx();
  typedef bit [7:0] payload_t;
  payload_t payload = payload_t'(lfsr_range(2**8-1));

  bit [10:0] packet = {
    1'b0, // start bit
    {<<{payload}}, // payload lsb-first
    ~^payload, // odd-parity
    1'b1 // stop bit
  };

  if (lfsr_range(1,0)) begin
    $display("[%0t] [DEVICE] Bus idle: skipping tx", $time);
    return;
  end

  $display("[%0t] [DEVICE] Bus idle: sending '0b%b' as '0b%b'",
    $time, payload, packet);

  // Starting at a dev_clk posedge
  foreach (packet[i]) begin

    // Generate bit 'dev_t1' before the falling edge
    #(dev_tclk/2-dev_t1);
    $display("[%0t] [DEVICE] ps2_dat <= %b (i = %0d)", $time, packet[i], i);
    dev_ps2_dat <= packet[i];

    // Check for inhibit before generating a ps2_clk falling edge
    @(negedge dev_clk);
    if (!ps2_clk) begin
      $display("[%0t] [DEVICE] Clock collision", $time);
      dev_ps2_dat <= '1;
      dev_ps2_clk <= '1;
      return;
    end

    // Generate the ps2_clk falling edge
    dev_ps2_clk <= '0;

    // Generate the ps2_clk rising edge
    @(posedge dev_clk);
    dev_ps2_clk <= '1;

  end

endtask

endmodule
