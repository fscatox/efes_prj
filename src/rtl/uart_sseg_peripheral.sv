/**
 * File          : uart_sseg_peripheral.sv
 * Author        : Fabio Scatozza <s315216@studenti.polito.it>
 * Date          : 13.12.2024
 * Description   : 7-Segment display peripheral with UART interface.
 *
 *                 The peripheral receives a UART character stream and displays it on a
 *                 set of NDISP 7-segment displays. The characters are mapped to the
 *                 displays in a circular fashion, starting from the display with the
 *                 highest index (NDISP-1) down to 0 if HIGH_FIRST is set; otherwise, the
 *                 mapping starts from display 0 up to display NDISP-1.
 *                 The MSB of each character acts as a reset: when it is set, all
 *                 displays are turned off, and the mapping restarts from the first
 *                 index of the sequence (NDISP-1 if HIGH_FIRST, or 0).
 *
 *                     7  6                        0
 *                    -------------------------------
 *                   |   |                           |
 *                   |CLR|           SEG             |
 *                   |   |                           |
 *                    -------------------------------
 *
 *                 When the transmission completes without frame or parity errors, the
 *                 receive strobe signal pulses high for one clock cycle. An one-hot
 *                 counter acts as display pointer, which is reset if the CLR bit is set.
 *                 Otherwise, the pointed display is driven with the SEG character field.
 */

module uart_sseg_peripheral #(
  parameter int unsigned NDISP = 6,
  parameter bit HIGH_FIRST = 1, // 1: NDISP-1 downto 0, 0: 0 up to NDISP-1
  parameter bit SEG_ON = 0, // segment turns on driving SEG_ON

  // UART configuration
  parameter bit PARITY = 1, // ON = 1, OFF = 0
  parameter bit PARITY_TYPE = 0, // EVEN = 0, ODD = 1
  parameter int unsigned NSTOP = 1,

  // Baud rate configuration
  parameter real FCLK_HZ = 50e6,
  parameter real FUART_HZ = 115200, // Effective: TimFclkHz/round(TimFclkHz/FUART_HZ)
  parameter int unsigned TIM_PSC = 30 // TimFclkHz = FCLK_HZ/(TIM_PSC+1)
) (
  input var logic clk,
  input var logic rst_n,
  input var logic urx,

  output logic [6:0] sseg[NDISP]
);

localparam bit [NDISP-1:0] OneHotReset = NDISP'(1) << (HIGH_FIRST ? NDISP-1 : 0);
localparam bit [6:0] SegReset = ~{7{SEG_ON}};

typedef struct packed {
  logic clr;
  logic [6:0] seg;
} char_t;

char_t char;
logic valid, rx_strobe;
logic frame_error, parity_error;
logic [NDISP-1:0] one_hot;

assign rx_strobe = valid & ~(frame_error|parity_error);

// One-hot ring counter
always_ff @(posedge clk, negedge rst_n)
  if (!rst_n)
    one_hot <= OneHotReset;
  else if (rx_strobe) begin
    if (char.clr)
      one_hot <= OneHotReset;
    else
      one_hot <= HIGH_FIRST ? {one_hot[0], one_hot[NDISP-1:1]} :
                              {one_hot[NDISP-2:0], one_hot[NDISP-1]};
  end

// Display FF banks
genvar i;
generate

  for (i = 0; i < NDISP; i++) begin : gen_ff_bank
    always_ff @(posedge clk, negedge rst_n)
      if (!rst_n)
        sseg[i] <= SegReset;
      else if (rx_strobe & (char.clr | one_hot[i]))
        sseg[i] <= char.clr ? SegReset : char.seg;
  end
 
endgenerate

uart_rx #(
  .NCHAR(8),
  .PARITY(PARITY),
  .PARITY_TYPE(PARITY_TYPE),
  .NSTOP(NSTOP),
  .FCLK_HZ(FCLK_HZ),
  .FUART_HZ(FUART_HZ),
  .TIM_PSC(TIM_PSC)
) uart_rx_i (.*);

endmodule

