/**
 * File          : uart_rx.sv
 * Author        : Fabio Scatozza <s315216@studenti.polito.it>
 * Date          : 12.12.2024
 * Description   : UART receiver with programmable frame format:
 *                   - NCHAR character bits
 *                   - optional EVEN or ODD parity
 *                   - NSTOP stop bits
 *                 The serial line is synchronized into the system clock domain with
 *                 a flip-flop chain of configurable length; typical oversampling factors
 *                 are 8 or 16, which should limit the selection of the baud rate. The
 *                 baud rate is fixed and is configured at compile time:
 *                   f_tim = f_clk / (TIM_PSC+1)
 *                   f_uart = f_tim / (BIT_TIME_TICKS+1)
 *                 When the transmission completes, the valid signal pulses high for one
 *                 clock cycle, while the flags and the rx_data output are updated. The
 *                 frame error is detected when a STOP bit is not recognized at the
 *                 expected time; the parity error is detected when the frame carries the
 *                 parity bit and the parity check fails.
 */

`include "math.svh"

module uart_rx #(
  // Frame configuration
  parameter int unsigned NCHAR = 8,
  parameter bit PARITY = 1, // ON = 1, OFF = 0
  parameter bit PARITY_TYPE = 0, // EVEN = 0, ODD = 1
  parameter int unsigned NSTOP = 1,

  // Baud rate configuration
  parameter real FCLK_HZ = 50e6,
  parameter real FUART_HZ = 115200,
  parameter int unsigned TIM_PSC = 30,

  // Clock domain crossing
  parameter int unsigned SYNC_STAGES = 2
) (
  input var logic clk,
  input var logic rst_n,

  output logic [NCHAR-1:0] rx_data,
  output logic valid,
  output logic frame_error,
  output logic parity_error,

  input var logic uart_rx
);

// Timer reload values
localparam real TimFclkHz = FCLK_HZ/(TIM_PSC+1);
localparam longint unsigned BitTimeTicks = `ROUND(TimFclkHz/FUART_HZ)-1;
localparam longint unsigned HalfBitTimeTicks =`ROUND(0.5*TimFclkHz/FUART_HZ)-1;

// Register definitions
localparam int unsigned TimNbit = $clog2(BitTimeTicks+1);
localparam int unsigned PscNbit = $clog2(TIM_PSC+1);
localparam int unsigned FrameSize = 1 + NCHAR + PARITY + NSTOP;

logic [SYNC_STAGES+1:1] urx_q;
logic urx_edge;

logic [FrameSize-1:0] shreg_q;
logic pbit;

logic [TimNbit-1:0] tim_cnt;
logic [PscNbit-1:0] psc_cnt;
logic psc_tc;

// status signals to the fsm
logic urx_negedge;
logic tim_tc;
logic start_bit;
logic pbit_update;

// control signals from the fsm
logic shreg_clk_en;
logic shreg_shift_pre_n;
logic pbit_clk_en;
logic pbit_mux;
logic out_clk_en;

// Synchronizer to mitigate metastability
// and ff for posedge/negedge detection
always_ff @(posedge clk, negedge rst_n) begin
  if (!rst_n)
    urx_q <= '1;
  else
    urx_q <= {urx_q[SYNC_STAGES:1], uart_rx};
end

assign urx_negedge = ~urx_q[SYNC_STAGES] & urx_q[SYNC_STAGES+1];
assign urx_edge = urx_q[SYNC_STAGES] ^ urx_q[SYNC_STAGES+1];

// Down-counter with prescaler and auto-reload
// All transitions on the serial line are used for resynchronizing the sampling instant
// (urx_edge triggers the reload, which restarts the down count of HALF_BIT_TIME). When
// the terminal count is reached (down to zero), the down count for ONE_BIT_TIME restarts.

// wraps around after TIM_PSC+1 cycles
always_ff @(posedge clk)
  if (urx_edge | psc_tc)
    psc_cnt <= '0;
  else
    psc_cnt <= psc_cnt + PscNbit'(1);

// wraps around after BitTimeTicks+1, unless it is reloaded to HalfBitTimeTicks
// (+'1 to avoid synthesizing an TimNbit+1 adder)
always_ff @(posedge clk)
  if (urx_edge)
    tim_cnt <= TimNbit'(HalfBitTimeTicks);
  else if (psc_tc)
    tim_cnt <= tim_tc ? TimNbit'(BitTimeTicks) : (tim_cnt + TimNbit'('1));

assign psc_tc = (psc_cnt == PscNbit'(TIM_PSC));
assign tim_tc = psc_tc & !tim_cnt;


// Right-shift register for sampling the incoming bits
// When the transmission starts, the shift register is preset at '1:
//   - the transmission ends when the START bit 0 reaches position 0
//   - the parity bit is updated by XORing the incoming bits, from START up to PARITY (the
//   result is the outcome of the parity check)
shift_reg #(.NBIT(FrameSize), .LEFT_RIGHT_N(0)) shreg_i (
  .clk,
  .clk_en(shreg_clk_en),
  .shift_load_n(shreg_shift_pre_n),
  .si(urx_q[SYNC_STAGES+1]),
  .d('1),
  .q(shreg_q)
);

assign start_bit = shreg_q[0];
assign pbit_update = &shreg_q[NSTOP:0];

always_ff @(posedge clk)
  if (out_clk_en) begin
    rx_data <= shreg_q[1 +: NCHAR];
    frame_error <= ~&shreg_q[FrameSize-1 -: NSTOP];
  end

generate // required in some synthesis tool
  if (!PARITY)
    assign parity_error = '0;

  // Parity computed dynamically, XORing the incoming bits
  else begin : gen_parity
    always_ff @(posedge clk)
      if (pbit_clk_en)
        pbit <= pbit_mux ? PARITY_TYPE : (pbit^urx_q[SYNC_STAGES+1]);

    always_ff @(posedge clk)
      if (out_clk_en)
        parity_error <= pbit;
  end
endgenerate

// FSM
uart_rx_fsm fsm_i (.*);

endmodule
