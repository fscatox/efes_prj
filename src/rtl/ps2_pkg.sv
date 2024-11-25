/**
 * File              : ps2_pkg.sv
 * Author            : Fabio Scatozza <s315216@studenti.polito.it>
 * Date              : 22.11.2024
 * Description       : PS/2 controller custom types
 */

package ps2_pkg;

  typedef struct packed {
    logic frame_error; // rx: missing start/stop; tx: missing ack
    logic parity_error; // rx: odd-parity bit wrong
    logic clk_timeout; // rx, tx: watchdog expired waiting for a clock negedge
    logic rqst_timeout; // tx: watchdog expired waiting for clock generation
  } flags_t;

  typedef struct packed {
    logic ps2_clk_negedge;
    logic ps2_dat;

    logic tim_tc;

    logic bitcnt_11, bitcnt_10, bitcnt_9, bitcnt_1;
    logic rx_ferr;
    logic perr;
    logic dbits;
  } status_t;

  typedef struct packed {
    logic [1:0] ps2_dat_mux;
    logic odreg_clk_en;

    logic tim_clk_en;
    logic tim_clear;
    logic [1:0] tc_mux;

    logic bitcnt_clk_en;
    logic bitcnt_clear;

    logic [1:0] pbit_mux;

    logic shreg_clk_en;
    logic shreg_shift_load_n;
    logic idreg_clk_en;
  } ctrl_t;

endpackage
