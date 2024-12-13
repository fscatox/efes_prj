/**
 * File          : tb_uart_tx_pkg.sv
 * Author        : Fabio Scatozza <s315216@studenti.polito.it>
 * Date          : 13.12.2024
 */

interface uart_if;
  bit to_rx;
endinterface

package tb_uart_tx_pkg;

  timeunit 1ns;
  timeprecision 1ns;

  import tb_utils_pkg::*;

  class UartFrame #(
    parameter int unsigned NCHAR = 8,
    parameter bit PARITY = 1, // ON = 1, OFF = 0
    parameter bit PARITY_TYPE = 0, // EVEN = 0, ODD = 1
    parameter int unsigned NSTOP = 1
  );

    localparam int unsigned FrameSize = 1 + NCHAR + PARITY + NSTOP;

    typedef bit [NCHAR-1:0] char_t;
    typedef bit [NSTOP-1:0] stop_t;
    typedef bit [FrameSize-1:0] raw_frame_t;

    stop_t stop;
    bit pbit;
    char_t char;

    // replaces ::randomize() for svverifcation license limitations
    function void lfsr_randomize();
      char = lfsr_range(2**NCHAR-1);
      pbit = ^{PARITY_TYPE, char};
      stop = '1;
    endfunction

    function raw_frame_t pack();
      return PARITY ? {stop, pbit, char, 1'b0} :
                      {stop, char, 1'b0};
    endfunction

    function string toStr();
      return $sformatf("char = h%h, pbit = %b, stop = %b", char, pbit, stop);
    endfunction

  endclass


  class UartTx #(
    type uframe_t = UartFrame,
    parameter real FUART_HZ = 115200
  );

    static time UartTclk = 1e9/FUART_HZ;

    // Inter-object analysis communication
    typedef struct {
      uframe_t uframe;
      bit frame_error;
      bit parity_error;
    } apkt_t;
    typedef mailbox #(apkt_t) mbox_t;

    virtual uart_if uart;
    mbox_t mbox;

    function new(virtual uart_if _uart, mbox_t _mbox = null);
      uart = _uart;
      mbox = _mbox;

      // idle bus
      uart.to_rx = 1;
    endfunction

    task tx(uframe_t uframe);
      // type(uframe_t::raw_frame_t) makes some simulators crash ?
      bit [uframe_t::FrameSize-1:0] raw_frame;

      // reverse bit order for transmission
      raw_frame = {<<{uframe.pack()}};
      log("UartTx", $sformatf("Transmitting [%s] as '%b'", uframe.toStr(), raw_frame));

      foreach (raw_frame[i]) begin
        uart.to_rx <= raw_frame[i];
        #UartTclk;
      end

    endtask

    task run(bit inject_errors = 0);
      uframe_t tpl = new();

      forever begin
        apkt_t apkt;
        uframe_t uframe;

        tpl.lfsr_randomize();
        uframe = new tpl;

        apkt = '{
          uframe: uframe,
          frame_error: 0,
          parity_error: 0
        };

        if (inject_errors) begin
          if (lfsr_range(99) < 10) begin // frame error probability
            log("UartTx", "Injecting frame error");
            uframe.stop[lfsr_range(uframe_t::NSTOP-1)] = '0;
            apkt.frame_error = 1;
          end
          if (uframe_t::PARITY & (lfsr_range(99) < 20)) begin // parity error
            log("UartTx", "Injecting parity error");
            uframe.pbit ^= 1;
            apkt.parity_error = 1;
          end
        end

        if (mbox != null)
          mbox.put(apkt);

        tx(uframe);

        // revert any frame error
        uart.to_rx = '1;
        #(lfsr_range(UartTclk, 1));

      end
    endtask

  endclass

endpackage

