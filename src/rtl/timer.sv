/**
 * File              : timer.sv
 * Author            : Fabio Scatozza <s315216@studenti.polito.it>
 * Date              : 22.11.2024
 * Description       : generic timer with fixed prescaler, clock enable and synchronous clear
 */

module timer #(
  parameter int unsigned PSC = 249, // clk frequency divided by (PSC+1)
  parameter int unsigned NBIT = 8
) (
  input var logic clk,
  input var logic clk_en,
  input var logic clear,

  output logic psc_tc,
  output logic [NBIT-1:0] cnt
);

localparam int unsigned PscNbit = $clog2(PSC+1);
logic [PscNbit-1:0] psc_cnt;

assign psc_tc = (psc_cnt == PscNbit'(PSC));

always_ff @(posedge clk)
  if (clk_en)
    // wraps around after PSC+1 cycles
    if (clear | psc_tc)
      psc_cnt <= '0;
    else
      psc_cnt <= psc_cnt + PscNbit'(1);

always_ff @(posedge clk)
  if (clk_en)
    if (clear)
      cnt <= '0;
    else if (psc_tc)
      cnt <= cnt + NBIT'(1);

endmodule
