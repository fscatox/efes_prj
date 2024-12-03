/**
 * File              : ps2_controller.sv
 * Author            : Fabio Scatozza <s315216@studenti.polito.it>
 * Date              : 22.11.2024
 * Description       : PS/2 controller
 *                     On reset or while the controller is not enabled, the PS/2 bus is inhibited by
 *                     driving the clock line low.
 *
 *                     If the controller is enabled and there is no pending transmission request,
 *                     the PS/2 bus is monitored for transmissions from the device.
 *                       - In case of successful reception, the valid signal is asserted for a clock
 *                       cycle and the received byte is latched on the output.
 *                       - In case of errors or timeouts, the valid signal rises and the status
 *                       flags are updated. While in this error state, the PS/2 bus is inhibited.
 *                       Resuming normal operation requires to cycle the enable signal.
 *
 *                     If the controller is enabled and a transmission is requested, the data to be
 *                     sent is sampled on the same clock edge.
 *                       - In case of successful transmission, the valid signal is asserted and is
 *                       kept high until the transmission request is deasserted. While waiting for
 *                       the handshake to complete, the PS/2 bus is inhibited.
 *                       - In case of errors or timeouts, the behavior is the same as in reception.
 */

module ps2_controller #(
  parameter int unsigned SYNC_STAGES = 2,
  parameter real FCLK_HZ = 50e6,
  parameter int unsigned TIM_PSC = 249 // tim_fclk = fclk/(PSC+1) = 200 kHz, for 50 MHz clock
) (
  input var logic clk,
  input var logic rst_n,
  input var logic en,

  input var logic tx_rqst,
  input var logic [7:0] tx_data,

  output logic valid,
  output logic [7:0] rx_data,
  output ps2_pkg::flags_t flags,

  inout tri ps2_clk,
  inout tri ps2_dat
);

// IO interface logic
logic ps2_clk_od, ps2_dat_od;
logic [SYNC_STAGES-1:0] ps2_clk_sync, ps2_dat_sync;

// DP/FSM interconnect
ps2_pkg::status_t status;
ps2_pkg::ctrl_t ctrl;

// FF synchronizers to mitigate metastability
always_ff @(posedge clk, negedge rst_n) begin
  if (!rst_n) begin
    ps2_clk_sync <= '1;
    ps2_dat_sync <= '1;
  end
  else begin
    ps2_clk_sync <= {ps2_clk_sync[SYNC_STAGES-2:0], ps2_clk};
    ps2_dat_sync <= {ps2_dat_sync[SYNC_STAGES-2:0], ps2_dat};
  end
end

// tristate buffers to drive the PS/2 open drain bus
assign ps2_clk = ps2_clk_od ? 'Z : '0;
assign ps2_dat = ps2_dat_od ? 'Z : '0;

ps2_dp #(.FCLK_HZ(FCLK_HZ), .PSC(TIM_PSC)) ps2_dp_i (
  .clk,
  .rst_n,
  .tx_data,
  .rx_data,
  .ps2_clk(ps2_clk_sync[SYNC_STAGES-1]),
  .ps2_dat(ps2_dat_sync[SYNC_STAGES-1]),
  .ps2_dat_od,
  .ctrl,
  .status
);

ps2_fsm ps2_fsm_i (
  .clk,
  .rst_n,
  .en,
  .tx_rqst,
  .valid,
  .flags,
  .status,
  .ctrl,
  .ps2_clk_od
);

endmodule
