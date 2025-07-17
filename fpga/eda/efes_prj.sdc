# File          : efes_prj.sdc
# Author        : Fabio Scatozza <s315216@studenti.polito.it>
# Date          : 16.12.2024

## Timing data

# On-board 50 MHz oscillator
set sys_tclk 20

# External SPI clock (MCU master, 16 MHz)
set spi_tclk 62.5
set spi_dtclk 3

## Clock inputs

# (neglect source latency, uncertainty)
create_clock -period $sys_tclk [get_ports clk] -name clk

# (neglect source latency)
create_clock -period $spi_tclk [get_ports sclk] -name sclk
set_clock_uncertainty -to sclk $spi_dtclk

# Determine internal clock uncertainties
derive_clock_uncertainty

## IO constraints 
# (neglect board delays)

# Asynchronous inputs
set_false_path -from [get_ports async_rst_n] -to *
set_false_path -from [get_ports sseg_urx] -to *

# Exclude timing paths to 7-seg displays
set_false_path -from * -to [get_ports sseg_out*]

## Clock domain crossing
set_false_path -from [get_clocks clk] -to [get_clocks sclk]
set_false_path -from [get_clocks sclk] -to [get_clocks clk]

