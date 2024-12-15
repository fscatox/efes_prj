/**
 * File          : arst_sync.sv
 * Author        : Fabio Scatozza <s315216@studenti.polito.it>
 * Date          : 14.12.2024
 * Description   : Asynchronous reset synchronizer (FPGA design recommendation).
 *                 The reset is asynchronously asserted and synchronously deasserted.
 *
 *                 Exclude unnecessary input timing reports generated when specifying
 *                 an input delay on the reset pin:
 *
 *                   `set_false_path -from [get_ports {reset_n}] -to [all_registers]`
 */

module arst_sync #(
  int unsigned SYNC_STAGES = 4
) (
  input var logic clk,
  input var logic async_rst_n,
  output logic rst_n
);

logic [SYNC_STAGES:1] sync_rst_n;

always_ff @(posedge clk, negedge async_rst_n)
  if (!async_rst_n)
    sync_rst_n <= '0; // assertion
  else
    sync_rst_n <= {sync_rst_n[SYNC_STAGES-1:1], 1'b1}; // synchronous deassertion

assign rst_n = sync_rst_n[SYNC_STAGES];

endmodule

