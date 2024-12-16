/**
 * File              : rshift_reg.sv
 * Author            : Fabio Scatozza <s315216@studenti.polito.it>
 * Date              : 22.11.2024
 * Description       : fixed direction shift-capable register with clock enable
 */

module shift_reg #(
  parameter int unsigned NBIT = 8,
  parameter bit LEFT_RIGHT_N = 0 // serial input loaded in (LEFT_RIGHT_N ? LSB : MSB)
) (
  input var logic clk,
  input var logic clk_en,
  input var logic shift_load_n,

  input var logic si,
  input var logic [NBIT-1:0] d,

  output logic [NBIT-1:0] q
);

always_ff @(posedge clk)
  if (clk_en)
    if (shift_load_n)
      q <= LEFT_RIGHT_N ? {q[NBIT-2:0], si} : {si, q[NBIT-1:1]};
    else
      q <= d;

endmodule
