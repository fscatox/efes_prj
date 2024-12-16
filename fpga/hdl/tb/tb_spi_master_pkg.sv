/**
 * File          : tb_spi_master_pkg.sv
 * Author        : Fabio Scatozza <s315216@studenti.polito.it>
 * Date          : 03.12.2024
 */

interface spi_if;
  bit ss_n, sclk;
  bit mosi;
  tri miso;

  modport master (
    input miso,
    output ss_n, sclk, mosi
  );
endinterface

package tb_spi_master_pkg;

  timeunit 1ns;
  timeprecision 1ns;

  import tb_utils_pkg::*;

  class SpiMaster #(
    parameter int unsigned NBIT = 8,
    parameter bit CPOL = 0,
    parameter bit CPHA = 0,
    parameter time TSCLK = 50ns, // 20 MHz clock
    parameter time TLEAD_SSN = 70ns // from ss_n negedge to leading sclk edge
  );

    // Inter-object analysis communication
    typedef bit [NBIT-1:0] data_t;
    typedef struct {
      data_t tx;
      data_t rx;
    } pkt_t;
    typedef mailbox #(pkt_t) mbox_t;

    virtual spi_if.master spi;
    mbox_t mbox;

    function new(virtual spi_if.master _spi, mbox_t _mbox = null);
      spi = _spi;
      mbox = _mbox;

      // idle bus
      spi.ss_n = '1;
      spi.sclk = CPOL;
    endfunction

    task transfer(input data_t tx_data, output data_t rx_data);
      spi.ss_n <= '0;
      if (!CPHA)
        spi.mosi <= tx_data[NBIT-1];
      else
        spi.mosi <= 'x; // generated on the leading edge, right now don't care
      log("SpiMaster", $sformatf("SPI begins: tx_data = h%h (CPHA = %b)", tx_data, CPHA));

      // delay for the slave to handle synchronization across clock domains
      #TLEAD_SSN;

      foreach (tx_data[i]) begin
        // leading sclk edge
        spi.sclk <= ~spi.sclk;
        if (!CPHA)
          rx_data[i] = spi.miso;
        else
          spi.mosi <= tx_data[i];
        #TSCLK;

        // trailing sclk edge
        spi.sclk <= ~spi.sclk;
        if (!CPHA)
          spi.mosi <= !i ? 'x : tx_data[i-1]; // after last sampling edge, don't care
        else
          rx_data[i] = spi.miso;
        #TSCLK;
      end

      spi.ss_n <= '1;
      log("SpiMaster", $sformatf("SPI ends: rx_data = h%h", rx_data));

    endtask

    task run();
      data_t tx_data, rx_data;

      forever begin
        tx_data = lfsr_range(2**NBIT-1);

        #(TSCLK+lfsr_range(TSCLK-1)); // delay before starting an exchange
        transfer(tx_data, rx_data);

        if (mbox != null)
          mbox.put('{tx: tx_data, rx: rx_data});
      end
    endtask

  endclass

endpackage
