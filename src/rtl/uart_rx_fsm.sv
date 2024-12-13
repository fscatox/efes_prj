/**
 * File          : uart_rx_fsm.sv
 * Author        : Fabio Scatozza <s315216@studenti.polito.it>
 * Date          : 12.12.2024
 * Description   : UART receiver fsm
 */

module uart_rx_fsm (
  input var logic clk,
  input var logic rst_n,
  output logic valid,

  input var logic urx_negedge,
  input var logic tim_tc,
  input var logic start_bit,
  input var logic pbit_update,

  output logic shreg_clk_en,
  output logic shreg_shift_pre_n,
  output logic pbit_clk_en,
  output logic pbit_mux,
  output logic out_clk_en
);

typedef enum logic [1:0] {
  IDLE, DT_SAMPLE, EVAL, DONE
} state_t;

state_t state, next_state;

//
// state register
//

always_ff @(posedge clk, negedge rst_n)
  if (!rst_n)
    state <= IDLE;
  else
    state <= next_state;

//
// next state logic
//

always_comb begin
  case (state)

    IDLE:
      next_state = !urx_negedge ? IDLE : DT_SAMPLE;

    DT_SAMPLE:
      next_state = !tim_tc ? DT_SAMPLE : EVAL;

    EVAL:
      next_state = start_bit ? DT_SAMPLE : DONE;

    default: // DONE
      next_state = IDLE;

  endcase
end

//
// output logic
//

always_comb begin

  //
  // defaults
  //

  shreg_clk_en = '0;
  shreg_shift_pre_n = '0;
  pbit_clk_en = '0;
  pbit_mux = '0;
  out_clk_en = '0;
  valid = '0;

  case (state)

    default: begin // IDLE
      shreg_clk_en = '1;
      pbit_clk_en = '1;
      pbit_mux = '1;
    end

    DT_SAMPLE:
      if (tim_tc) begin
        shreg_clk_en = '1;
        shreg_shift_pre_n = '1;
        pbit_clk_en = pbit_update;
      end

    EVAL:
      if (!start_bit)
        out_clk_en = '1;

    DONE:
      valid = '1;

  endcase
end

endmodule
