/**
 * File              : ps2_dp.sv
 * Author            : Fabio Scatozza <s315216@studenti.polito.it>
 * Date              : 22.11.2024
 * Description       : PS/2 controller datapath
 */

`include "math.svh"

module ps2_dp #(
  parameter real FCLK_HZ = 50e6,
  parameter int unsigned PSC = 249, // tim_fclk = fclk/(PSC+1) = 200 kHz, for 50 MHz reference clock

  parameter real SAMPLING_DELAY_S = 15e-6,
  parameter real CLK_INHIBIT_S = 100e-6,
  parameter real WATCHDOG_EDGE_S = 110e-6,
  parameter real WATCHDOG_RQST_S = 15e-3
) (
  input var logic clk,
  input var logic [7:0] tx_data,
  output logic [7:0] rx_data,

  input var logic ps2_clk,
  input var logic ps2_dat,
  output logic ps2_dat_od,

  // DP/FSM interconnect
  input var ps2_pkg::ctrl_t ctrl,
  output ps2_pkg::status_t status
);

//
// Threshold computations
//    i) 15 us as a sampling delay
//   ii) 100 us for inhibiting the clock while in the request-to-send configuration
//  iii) 110 us as a watchdog while waiting for a clock negedge
//   iv) 15 ms as a watchodg while waiting for the clock following the request-to-send
//
localparam real TimFclkHz = FCLK_HZ/(PSC+1);
localparam longint unsigned SamplingDelayTc = `CEIL(TimFclkHz*SAMPLING_DELAY_S)-1;
localparam longint unsigned ClkInhibitTc = `CEIL(TimFclkHz*CLK_INHIBIT_S)-1;
localparam longint unsigned WatchdogEdgeTc = `CEIL(TimFclkHz*WATCHDOG_EDGE_S)-1;
localparam longint unsigned WatchdogRqstTc = `CEIL(TimFclkHz*WATCHDOG_RQST_S)-1;

//
// Register sizing
//
localparam int unsigned TimNbit = $clog2(WatchdogRqstTc+1); // max duration to measure

localparam int unsigned PayloadSize = 8;
localparam int unsigned MaxFrameSize = 12;
localparam int unsigned FrameCntSize = $clog2(MaxFrameSize);

///
logic ps2_clk_dl;
logic ps2_dat_dl;

logic [PayloadSize-1:0] shreg_q;
logic pbit;

logic tim_psc_tc;
logic [TimNbit-1:0] tim_cnt;
logic [TimNbit-1:0] tim_cmp;

logic [FrameCntSize-1:0] bitcnt_cnt;
logic bitcnt_0;

//
// negedge detector for ps2_clk
// ps2_dat is delayed accordingly
//
always_ff @(posedge clk) begin
  ps2_clk_dl <= ps2_clk;
  ps2_dat_dl <= ps2_dat;
end

assign status.ps2_clk_negedge = ps2_clk_dl & ~ps2_clk;
assign status.ps2_dat = ps2_dat_dl;

//
// multiplexed drivers of ps2_dat
//
always_ff @(posedge clk) begin
  if (ctrl.odreg_clk_en) begin
    case (ctrl.ps2_dat_mux)
      default : ps2_dat_od <= '1; // 'z
      2'b01   : ps2_dat_od <= '0; // bus inhibited
      2'b10   : ps2_dat_od <= shreg_q[0]; // shifting data bits
      2'b11   : ps2_dat_od <= pbit; // shifting parity bit
    endcase
  end
end

//
// timer and tc comparator
//
timer #(.PSC(PSC), .NBIT(TimNbit)) tim_i (
  .clk,
  .clk_en(ctrl.tim_clk_en),
  .clear(ctrl.tim_clear),
  .psc_tc(tim_psc_tc),
  .cnt(tim_cnt)
);

always_comb begin
  case (ctrl.tc_mux)
    default : tim_cmp = TimNbit'(SamplingDelayTc);
    2'b01   : tim_cmp = TimNbit'(ClkInhibitTc);
    2'b10   : tim_cmp = TimNbit'(WatchdogEdgeTc);
    2'b11   : tim_cmp = TimNbit'(WatchdogRqstTc);
  endcase

  // the tc pulse spans one (prescaler) clock cycle only
  status.tim_tc = (tim_cnt == tim_cmp) & tim_psc_tc;
end

//
// bits counter and comparison logic
//
up_counter #(.NBIT(FrameCntSize)) bitcnt_i (
  .clk,
  .clk_en(ctrl.bitcnt_clk_en),
  .clear(ctrl.bitcnt_clear),
  .cnt(bitcnt_cnt)
);

assign status.bitcnt_11 = bitcnt_cnt == FrameCntSize'(11);
assign status.bitcnt_10 = bitcnt_cnt == FrameCntSize'(10);
assign status.bitcnt_9 = bitcnt_cnt == FrameCntSize'(9);
assign status.bitcnt_1 = bitcnt_cnt == FrameCntSize'(1);

always_comb begin
  bitcnt_0 = bitcnt_cnt == '0;
  status.dbits = ~(status.bitcnt_10 | status.bitcnt_9 | bitcnt_0);
  status.rx_ferr = (bitcnt_0 & ps2_dat_dl) | (status.bitcnt_10 & ~ps2_dat_dl);
  status.perr = status.bitcnt_9 & (ps2_dat_dl != pbit);
end

//
// shift register for serial rx/tx operations
//
shift_reg #(.NBIT(PayloadSize), .LEFT_RIGHT_N(0)) shreg_i (
  .clk,
  .clk_en(ctrl.shreg_clk_en),
  .shift_load_n(ctrl.shreg_shift_load_n),
  .si(ps2_dat_dl),
  .d(tx_data),
  .q(shreg_q)
);

// rx_data registered output
always_ff @(posedge clk)
  if (ctrl.idreg_clk_en)
    rx_data <= shreg_q;

//
// sequential computation of pbit
//
always_ff @(posedge clk) begin
  case (ctrl.pbit_mux)
    default : pbit <= pbit;
    2'b01   : pbit <= '1; // initialization for odd parity
    2'b10   : pbit <= pbit ^ ps2_dat_dl; // of rx bits
    2'b11   : pbit <= pbit ^ shreg_q[0]; // of tx bits
  endcase
end

endmodule
