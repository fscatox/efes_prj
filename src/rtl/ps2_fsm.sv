/**
 * File              : ps2_fsm.sv
 * Author            : Fabio Scatozza <s315216@studenti.polito.it>
 * Date              : 22.11.2024
 * Description       : PS/2 controller fsm
 */

module ps2_fsm (
  input var logic clk,
  input var logic rst_n,
  input var logic en,
  input var logic tx_rqst,
  output logic valid,
  output ps2_pkg::flags_t flags,

  // DP/FSM interconnect
  input var ps2_pkg::status_t status,
  output ps2_pkg::ctrl_t ctrl,

  output logic ps2_clk_od
);

typedef enum logic [3:0] {
  INHIBIT,
  RX_IDLE, DT_SAMPLE, RX_WAIT, RX_DONE,
  GEN_RQST, TX_IDLE, GEN_DBIT, GEN_PARITY, GEN_STOP, DT_SAMPLE_ACK, TX_DONE,
  FERR, PERR, CLK_TO, RQST_TO
} state_t;

state_t state, next_state;

//
// state register
//
always_ff @(posedge clk, negedge rst_n)
  if (!rst_n)
    state <= INHIBIT;
  else
    state <= next_state;

//
// next state logic
//
// Notes: using functions to shorten the description prevents fsm detection on
// some simulators and synthesizers. Example:
//
//  RX_IDLE:
//    if (!rx_break_pattern_state(en, tx_rqst, next_state))
//      next_state = !status.ps2_clk_negedge ? RX_IDLE : DT_SAMPLE;
//

always_comb begin
  case (state)
    INHIBIT:
      if (!en)
        next_state = INHIBIT;
      else if (tx_rqst)
        next_state = GEN_RQST;
      else
        next_state = RX_IDLE;

    RX_IDLE, DT_SAMPLE, RX_WAIT, RX_DONE:
      // rx break pattern -->
      if (!en)
        next_state = INHIBIT;
      else if (tx_rqst)
        next_state = GEN_RQST;
      // <--

      else begin
        unique case (state)
          RX_IDLE:
            next_state = !status.ps2_clk_negedge ? RX_IDLE : DT_SAMPLE;

          DT_SAMPLE:
            if (!status.tim_tc)
              next_state = DT_SAMPLE;
            else if (status.rx_ferr)
              next_state = FERR;
            else if (status.perr)
              next_state = PERR;
            else if (!status.bitcnt_10)
              next_state = RX_WAIT;
            else
              next_state = RX_DONE;

          RX_WAIT:
            if (status.ps2_clk_negedge)
              next_state = DT_SAMPLE;
            else if (!status.tim_tc)
              next_state = RX_WAIT;
            else
              next_state = CLK_TO;

          RX_DONE:
            next_state = RX_IDLE;
        endcase
      end

    GEN_RQST, TX_IDLE, GEN_DBIT, GEN_PARITY, GEN_STOP, DT_SAMPLE_ACK, TX_DONE:
      // tx break pattern -->
      if (!en)
        next_state = INHIBIT;
      // <--

      else begin
        unique case (state)
          GEN_RQST:
              next_state = !status.tim_tc ? GEN_RQST : TX_IDLE;

          TX_IDLE:
            if (!status.ps2_clk_negedge)
              if (status.bitcnt_1)
                next_state = !status.tim_tc ? TX_IDLE : RQST_TO;
              else
                next_state = !status.tim_tc ? TX_IDLE : CLK_TO;
            else if (status.bitcnt_11)
              next_state = DT_SAMPLE_ACK;
            else if (status.bitcnt_10)
              next_state = GEN_STOP;
            else if (status.bitcnt_9)
              next_state = GEN_PARITY;
            else
              next_state = GEN_DBIT;

          GEN_DBIT, GEN_PARITY, GEN_STOP:
            next_state = TX_IDLE;

          DT_SAMPLE_ACK:
            if (!status.tim_tc)
              next_state = DT_SAMPLE_ACK;
            else if (status.ps2_dat)
              next_state = FERR;
            else
              next_state = TX_DONE;

          TX_DONE:
            next_state = tx_rqst ? TX_DONE : INHIBIT;
        endcase
      end

    FERR:
      next_state = en ? FERR : INHIBIT;

    PERR:
      next_state = en ? PERR : INHIBIT;

    CLK_TO:
      next_state = en ? CLK_TO : INHIBIT;

    RQST_TO:
      next_state = en ? RQST_TO : INHIBIT;

    default:
      next_state = INHIBIT;

  endcase
end

//
// output logic
//

function automatic logic rx_break_pattern_out (
  input logic en, input logic tx_rqst,
  inout logic tim_clk_en, inout logic tim_clear, inout logic shreg_clk_en
);

  if (en & tx_rqst) begin
    tim_clk_en = 1;
    tim_clear = 1;
    shreg_clk_en = 1;
  end

  return !en | tx_rqst; // true if jumped
endfunction

always_comb begin

  //
  // defaults
  //
  valid = '0;
  flags = '0;
  ctrl = '0;
  ctrl.odreg_clk_en = '1;
  ps2_clk_od = '1;

  case (state)
    INHIBIT: begin
      ps2_clk_od = '0;

      // mealy
      if (en & tx_rqst) begin
        ctrl.tim_clk_en = '1;
        ctrl.tim_clear = '1;
        ctrl.shreg_clk_en = '1;
      end
    end

    RX_IDLE: begin
      ctrl.pbit_mux = 2'b01;
      ctrl.bitcnt_clk_en = '1;
      ctrl.bitcnt_clear = '1;

      // mealy
      if (!rx_break_pattern_out(en, tx_rqst, ctrl.tim_clk_en, ctrl.tim_clear, ctrl.shreg_clk_en) &
        status.ps2_clk_negedge)
      begin
        ctrl.tim_clk_en = '1;
        ctrl.tim_clear = '1;
      end
    end

    DT_SAMPLE: begin
      ctrl.tim_clk_en = '1;

      // mealy
      if (!rx_break_pattern_out(en, tx_rqst, ctrl.tim_clk_en, ctrl.tim_clear, ctrl.shreg_clk_en) &
        status.tim_tc & !status.rx_ferr & !status.perr)
      begin
        ctrl.bitcnt_clk_en = '1;
        if (status.dbits) begin
          ctrl.shreg_clk_en = '1;
          ctrl.shreg_shift_load_n = '1;
          ctrl.pbit_mux = 2'b10;
        end
      end
    end

    RX_WAIT: begin
      ctrl.tim_clk_en = '1;
      ctrl.tc_mux = 2'b10;

      // mealy
      if (!rx_break_pattern_out(en, tx_rqst, ctrl.tim_clk_en, ctrl.tim_clear, ctrl.shreg_clk_en) &
          status.ps2_clk_negedge)
        ctrl.tim_clear = '1;
    end

    RX_DONE: begin
      valid = '1;
      ctrl.idreg_clk_en = '1;

      // mealy
      // workaround for void'() cast
      if(rx_break_pattern_out(en, tx_rqst, ctrl.tim_clk_en, ctrl.tim_clear, ctrl.shreg_clk_en));

    end

    PERR: begin
      ps2_clk_od = '0;
      valid = '1;
      flags.parity_error = '1;
    end

    GEN_RQST: begin
      ps2_clk_od = '0;
      ctrl.ps2_dat_mux = 2'b01;
      ctrl.pbit_mux = 2'b01;
      ctrl.tim_clk_en = '1;
      ctrl.tc_mux = 2'b01;
      ctrl.bitcnt_clk_en = '1;
      ctrl.bitcnt_clear = '1;

      // mealy
      if (en & status.tim_tc)
        ctrl.bitcnt_clear = '0;
    end

    TX_IDLE: begin
      ctrl.odreg_clk_en = '0;
      ctrl.tim_clk_en = '1;

      // mealy
      if (en) begin
        if (status.ps2_clk_negedge)
          ctrl.tim_clear = '1;
        else
          ctrl.tc_mux = status.bitcnt_1 ? 2'b11 : 2'b10;
      end
    end

    GEN_DBIT: begin
      ctrl.ps2_dat_mux = 2'b10;
      ctrl.pbit_mux = 2'b11;
      ctrl.shreg_clk_en = '1;
      ctrl.shreg_shift_load_n = '1;
      ctrl.tim_clk_en = '1;
      ctrl.bitcnt_clk_en = '1;
    end

    GEN_PARITY: begin
      ctrl.ps2_dat_mux = 2'b11;
      ctrl.tim_clk_en = '1;
      ctrl.bitcnt_clk_en = '1;
    end

    GEN_STOP: begin
      ctrl.tim_clk_en = '1;
      ctrl.bitcnt_clk_en = '1;
    end

    DT_SAMPLE_ACK:
      ctrl.tim_clk_en = '1;

    TX_DONE: begin
      ps2_clk_od = '0;
      valid = '1;
    end

    FERR: begin
      ps2_clk_od = '0;
      valid = '1;
      flags.frame_error = '1;
    end

    CLK_TO: begin
      ps2_clk_od = '0;
      valid = '1;
      flags.clk_timeout = '1;
    end

    RQST_TO: begin
      ps2_clk_od = '0;
      valid = '1;
      flags.rqst_timeout = '1;
    end

    default:
      ps2_clk_od = '0;

  endcase
end

endmodule
