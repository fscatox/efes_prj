/**
 * File              : up_counter.sv
 * Author            : Fabio Scatozza <s315216@studenti.polito.it>
 * Date              : 22.11.2024
 * Description       : generic up counter with clock enable and synchronous clear
 */

module up_counter #(
  parameter int unsigned NBIT = 8
) (
  input var logic clk,
  input var logic clk_en,
  input var logic clear,
  output logic [NBIT-1:0] cnt
);

always_ff @(posedge clk)
  if (clk_en)
    if (clear)
      cnt <= '0;
    else
      cnt <= cnt + NBIT'(1);

endmodule
