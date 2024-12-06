/**
 * File          : tb_ps2_device_pkg.sv
 * Author        : Fabio Scatozza <s315216@studenti.polito.it>
 * Date          : 29.11.2024
 */

interface ps2_if;
  tri1 clk, dat;
  bit clk_od, dat_od;

  // device drivers
  assign clk = clk_od ? 'z : '0;
  assign dat = dat_od ? 'z : '0;

  modport dev (
    input clk, dat,
    output clk_od, dat_od
  );
endinterface

package tb_ps2_device_pkg;

  timeunit 1ns;
  timeprecision 1ns;

  import tb_utils_pkg::*;

  function automatic string flagsToStr(ps2_pkg::flags_t flags);
    unique case (1'b1)
      flags.frame_error: return "frame error";
      flags.parity_error: return "parity error";
      flags.clk_timeout: return "clock timeout";
      flags.rqst_timeout: return "request-to-send timeout";
    endcase
  endfunction

  //
  // Device model
  // See: IBM PS/2 Reference Manuals, Keyboard/Auxiliary Device Controller, 1990
  // Available: http://www.mcamafia.de/pdf/pdfref.htm
  //

  class Ps2Device;

    const time tclk;
    const time ts; // from data transition to falling edge
    const time dt; // clock shift

    const time tclk_to;
    const time trqst_to;

    typedef bit [7:0] payload_t;
    typedef enum {
      NOERR,
      FERR_START,
      PERR,
      FERR_STOP,
      RX_CLK_TO_[0:9],
      RQST_TO,
      TX_CLK_TO_[0:9],
      FERR_ACK
    } err_t;

    virtual ps2_if.dev ps2;
    bit clk;

    function new(virtual ps2_if.dev _ps2, time _dt = 0);
      tclk = 50us;
      ts = lfsr_range(25,5)*1us;
      dt = _dt;
      tclk_to = 150us;
      trqst_to = 20ms;

      ps2 = _ps2;
      clk = '0;
    endfunction

    task run(err_t err);
      ps2.dat_od = '1;
      ps2.clk_od = '1;

      fork

        begin : clk_p
          #dt;
          forever #(tclk/2) clk <= ~clk;
        end

        forever begin
          @(posedge clk);
          log("Ps2Device", "Checking the bus state");

          if (ps2.clk) begin // not inhibited
            if (ps2.dat) // not a request-to-send
              _tx(err);
            else
              _rx(err);

            if (err != NOERR)
              err = err.next();
          end
        end

      join
    endtask

    task _tx(err_t err);
      payload_t py = payload_t'(lfsr_range(2**8-1));

      bit [10:0] pkt = {
        err != FERR_START ? 1'b0 : 1'b1, // start bit
        {<<{py}}, // payload lsb-first
        err != PERR ? ~^py : ^py, // odd-parity
        err != FERR_STOP ? 1'b1 : 1'b0 // stop bit
      };

      log("Ps2Device", $sformatf("Sending b%b as b%b (%s)", py, pkt, err.name()));
      foreach (pkt[i]) begin

        // Generate bit 't1' before the falling edge
        #(tclk/2-ts);
        ps2.dat_od <= pkt[i];
        log("Ps2Device", $sformatf("(i = %0d) ps2_dat <= %b", i, pkt[i]));

        // Check for inhibit at every falling edge, before generating
        // a transition on the bus
        @(negedge clk);
        if (!ps2.clk) begin
          log("Ps2Device", "Clock collision: aborting tx");
          ps2.dat_od <= '1;
          return;
        end

        // Generate the falling edge
        if (i != 10 & i == int'(err)-int'(RX_CLK_TO_0)) begin
          log("Ps2Device", $sformatf("(i = %0d) Forcing a clock timeout", i));
          ps2.dat_od <= '1;
          #tclk_to;
          return;
        end

        ps2.clk_od <= '0;

        // Generate the rising edge
        @(posedge clk);
        ps2.clk_od <= '1;

      end

      if (err == FERR_STOP)
        ps2.dat_od <= '1;

    endtask

    task _rx(err_t err);
      payload_t py;

      for (int i = 0; i < 11; i++) begin

        // Move to the sampling point
        #(tclk/4);

        if (i > 0 & i < 9) // data bit
          py[i-1] = ps2.dat;
        else if (i == 9 & ps2.dat != ~^py) begin
          log("Ps2Device", "Host parity error", 1);
        end
        else if (i == 10 & !ps2.dat) begin
          log("Ps2Device", "Host frame error", 1);
        end

        // Reverse roles for generating the acknowledgement
        if (i == 10 & err != FERR_ACK) begin
          ps2.dat_od <= '0;
        end

        // Generate the falling edge
        if (err == RQST_TO) begin
          log("Ps2Device", $sformatf("Forcing a rqst timeout (%s)", err.name()));
          #trqst_to;
          return;
        end
        if (i > 0 & i-1 == int'(err)-int'(TX_CLK_TO_0)) begin
          log("Ps2Device", $sformatf("Forcing a clk timeout (%s)", err.name()));
          ps2.dat_od <= '1;
          #tclk_to;
          return;
        end

        @(negedge clk);
        ps2.clk_od <= '0;

        // Generate the rising edge
        @(posedge clk);
        ps2.clk_od <= '1;

      end

      // Revert acknowledgement
      ps2.dat_od <= '1;
      log("Ps2Device", $sformatf("Rx done: b%b", py));
    endtask

  endclass

endpackage
