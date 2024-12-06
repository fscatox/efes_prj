/**
 * File          : tb_fifo_pkg.sv
 * Author        : Fabio Scatozza <s315216@studenti.polito.it>
 * Date          : 06.12.2024
 * Description   : Cycle-accurate golden model for sa_sc_fifo.sv
 */

package tb_fifo_pkg;

  import tb_utils_pkg::*;

  class GoldenFifo #(
    parameter int unsigned DATA_WIDTH = 8,
    parameter int unsigned ADDR_WIDTH = 8
  );

    typedef bit [DATA_WIDTH-1:0] data2_t;
    typedef logic [DATA_WIDTH-1:0] data4_t; // 'x to skip checker comparisons for don't care cases

    // ram model with fifo mgmt
    data2_t fifo[$:2**ADDR_WIDTH-1];

    // ram latency modeling
    bit rdw;

    // outputs
    data4_t q;
    bit empty;
    bit full;

    function new();
      reset();
    endfunction

    function void reset();
      fifo = {};
      rdw = 0;
      q = 'x; // show-ahead undefined
      empty = 1;
      full = 0;
    endfunction

    function void clock(bit sclr, bit we, bit ack, data2_t d);
      log("GoldenFifo", {
        "Inputs:\n",
        $sformatf("sclr = %b, we = %b, ack = %b, d = h%h\n", sclr, we, ack, d),
        "Init state:\n",
        $sformatf("rdw = %b, empty = %b, full = %b, q = h%h\n%s", rdw, empty, full, q, toStr())
      });

      if (sclr)
        reset();

      else begin

        if (empty & ack)
          log("GoldenFifo", "Bad driver: empty & ack", 1);
        if (full & we)
          log("GoldenFifo", "Bad driver: full & we", 1);

        if (rdw) begin // latency elapsed
          q = fifo[$];
          rdw = 0;
        end
        empty = !fifo.size();
        full = (fifo.size() == 2**ADDR_WIDTH);

        if (ack) begin
          void'(fifo.pop_back());
          q = fifo.size() ? fifo[$] : 'x; // show-ahead undefined when empty
          empty = !fifo.size();
          full = (fifo.size() == 2**ADDR_WIDTH);
        end

        if (we) begin
          if (!fifo.size()) begin
            log("GoldenFifo", "Read-during-write");
            q = 'x; // ignore embedded ram behavior for RDW
            empty = 1;
            rdw = 1;
          end
          fifo.push_front(d);
          full = (fifo.size() == 2**ADDR_WIDTH);
        end

      end

      log("GoldenFifo", {
        "Exit state:\n",
        $sformatf("rdw = %b, empty = %b, full = %b, q = h%h\n%s", rdw, empty, full, q, toStr())
      });
    endfunction

    function string toStr();
      string s = fifo.size() ? "" : "fifo: NULL\n";
      foreach (fifo[i])
        s = {s, $sformatf("fifo[%0d]: h%h\n", i, fifo[i])};
      return s;
    endfunction

  endclass

endpackage

