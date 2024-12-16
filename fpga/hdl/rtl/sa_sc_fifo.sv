/**
 * File          : sa_sc_fifo.sv
 * Author        : Fabio Scatozza <s315216@studenti.polito.it>
 * Date          : 05.12.2024
 * Description   : Show-ahead single-clock FIFO, inferring dual-port RAM.
 *
 *                 On reset (asynchronous or synchronous with sclr), the FIFO is
 *                 empty: the empty flag is set and there is no read data to
 *                 acknowledge. With the first write operation, the data is
 *                 stored in the FIFO and is made available at the q output
 *                 with a latency of 2 cycles. The assumption is that, following
 *                 the clock cycle in which the we signal is asserted, the
 *                 behavior of the embedded RAM for the read-during-write is
 *                 undefined (or to output the old data), thus an additional
 *                 cycle is needed for the written data to appear at the q
 *                 output. Accordingly, the empty flag is cleared with the same
 *                 latency of 2 clock cycles.
 *
 *                 In general, the design complies to the following latencies:
 *
 *                   we   --> q     : 2 cycles
 *                   we   --> empty : 2 cycles
 *                   we   --> full  : 1 cycle
 *
 *                   ack  --> q     : 1 cycle
 *                   ack  --> empty : 1 cycle
 *                   ack  --> full  : 1 cycle
 */

module sa_sc_fifo #(
  parameter int unsigned DATA_WIDTH = 8,
  parameter int unsigned ADDR_WIDTH = 8
) (
  input var logic clk,
  input var logic rst_n,
  input var logic sclr,

  input var logic [DATA_WIDTH-1:0] d,
  input var logic we,
  input var logic ack,

  output logic [DATA_WIDTH-1:0] q,
  output logic empty,
  output logic full
);


// Read/write pointers are 1 bit larger than the addresses to account for
// wrap-around when detecting empty and full conditions.
logic [ADDR_WIDTH:0] wptr, rptr;
logic [ADDR_WIDTH:0] wptr_next, rptr_next;
logic [DATA_WIDTH-1:0] ram[2**ADDR_WIDTH];

// The next pointer is computed in the same clock cycle when the we/ack signals
// are asserted, to update in time the registered empty and full flags.
assign wptr_next = wptr + (ADDR_WIDTH+1)'(we);
assign rptr_next = rptr + (ADDR_WIDTH+1)'(ack);

always_ff @(posedge clk, negedge rst_n)
  if (!rst_n) begin
    wptr <= '0;
    rptr <= '0;
  end
  else begin
    if (sclr) begin
      wptr <= '0;
      rptr <= '0;
    end
    else begin
      wptr <= wptr_next;
      rptr <= rptr_next;
    end
  end

// Empty
// we --> empty has 2 clock cycle latency (wptr is compared)
// ack --> empty has 1 clock cycle latency (rptr_next is compared)
always_ff @(posedge clk, negedge rst_n)
  if (!rst_n)
    empty <= '1;
  else begin
    if (sclr)
      empty <= '1;
    else
      empty <= wptr == rptr_next;
  end

// Full
// When wptr_next[ADDR_WIDTH-1:0] == rptr_next[ADDR_WIDTH-1:0], could be
// empty (same MSBs) or full (opposite MSBs).
always_ff @(posedge clk, negedge rst_n)
  if (!rst_n)
    full <= '0;
  else begin
    if (sclr)
      full <= '0;
    else
      full <= {~wptr_next[ADDR_WIDTH],wptr_next[ADDR_WIDTH-1:0]} == rptr_next;
  end

// Synchronous RAM with old-data RDW behavior
always @(posedge clk) begin
  // wptr: next location to write
  if (we)
    ram[wptr[ADDR_WIDTH-1:0]] <= d;

  // rptr: currently read location
  // ack terminates current read, thus the next location to read is rptr_next
  q <= ram[rptr_next[ADDR_WIDTH-1:0]];
end

endmodule
