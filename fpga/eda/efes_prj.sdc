# File          : efes_prj.sdc
# Author        : Fabio Scatozza <s315216@studenti.polito.it>
# Date          : 16.12.2024

## Clock inputs

# On-board 50 MHz oscillator
create_clock -period 20 [get_ports {clk}] -name clk

# External SPI clock (assuming maximum frequency of 10 MHz)
create_clock -period 100 [get_ports {sclk}] -name sclk

# Determine internal clock uncertainties
derive_clock_uncertainty

