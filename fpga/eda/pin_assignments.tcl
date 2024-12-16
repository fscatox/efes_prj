# File          : pin_assignments.tcl
# Author        : Fabio Scatozza <s315216@studenti.polito.it>
# Date          : 14.12.2024

#
# MCU interface
#

set_location_assignment PIN_Y17  -comment {GPIO[0][1]} -to async_rst_n
set_location_assignment PIN_AD17 -comment {GPIO[0][2]} -to sclk
set_location_assignment PIN_Y18  -comment {GPIO[0][3]} -to sseg_urx
set_location_assignment PIN_AK16 -comment {GPIO[0][4]} -to mosi
set_location_assignment PIN_AK19 -comment {GPIO[0][6]} -to ps2_ss_n
set_location_assignment PIN_AJ19 -comment {GPIO[0][7]} -to ps2_miso
set_location_assignment PIN_AJ17 -comment {GPIO[0][8]} -to adc_ss_n
set_location_assignment PIN_AJ16 -comment {GPIO[0][9]} -to adc_miso_forwarded

set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to async_rst_n
set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to sclk
set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to sseg_urx
set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to mosi
set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to ps2_ss_n
set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to ps2_miso
set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to adc_ss_n
set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to adc_miso_forwarded

#
# Board interface
#

# Clock
set_location_assignment PIN_AA16 -comment CLOCK2_50 -to clk
set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to clk

# PS/2 port
set_location_assignment PIN_AD7 -to ps2_clk
set_location_assignment PIN_AE7 -to ps2_dat

set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to ps2_clk
set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to ps2_dat

# ADC port
set_location_assignment PIN_AK2 -to adc_sclk
set_location_assignment PIN_AK4 -to adc_mosi
set_location_assignment PIN_AJ4 -to adc_ss_n_forwarded
set_location_assignment PIN_AK3 -to adc_miso

set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to adc_sclk
set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to adc_mosi
set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to adc_miso
set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to adc_ss_n_forwarded

# 7-Segment Display ports
set_location_assignment PIN_AE26 -to {sseg_out[0][0]}
set_location_assignment PIN_AE27 -to {sseg_out[0][1]}
set_location_assignment PIN_AE28 -to {sseg_out[0][2]}
set_location_assignment PIN_AG27 -to {sseg_out[0][3]}
set_location_assignment PIN_AF28 -to {sseg_out[0][4]}
set_location_assignment PIN_AG28 -to {sseg_out[0][5]}
set_location_assignment PIN_AH28 -to {sseg_out[0][6]}
set_location_assignment PIN_AJ29 -to {sseg_out[1][0]}
set_location_assignment PIN_AH29 -to {sseg_out[1][1]}
set_location_assignment PIN_AH30 -to {sseg_out[1][2]}
set_location_assignment PIN_AG30 -to {sseg_out[1][3]}
set_location_assignment PIN_AF29 -to {sseg_out[1][4]}
set_location_assignment PIN_AF30 -to {sseg_out[1][5]}
set_location_assignment PIN_AD27 -to {sseg_out[1][6]}
set_location_assignment PIN_AB23 -to {sseg_out[2][0]}
set_location_assignment PIN_AE29 -to {sseg_out[2][1]}
set_location_assignment PIN_AD29 -to {sseg_out[2][2]}
set_location_assignment PIN_AC28 -to {sseg_out[2][3]}
set_location_assignment PIN_AD30 -to {sseg_out[2][4]}
set_location_assignment PIN_AC29 -to {sseg_out[2][5]}
set_location_assignment PIN_AC30 -to {sseg_out[2][6]}
set_location_assignment PIN_AD26 -to {sseg_out[3][0]}
set_location_assignment PIN_AC27 -to {sseg_out[3][1]}
set_location_assignment PIN_AD25 -to {sseg_out[3][2]}
set_location_assignment PIN_AC25 -to {sseg_out[3][3]}
set_location_assignment PIN_AB28 -to {sseg_out[3][4]}
set_location_assignment PIN_AB25 -to {sseg_out[3][5]}
set_location_assignment PIN_AB22 -to {sseg_out[3][6]}
set_location_assignment PIN_AA24 -to {sseg_out[4][0]}
set_location_assignment PIN_Y23  -to {sseg_out[4][1]}
set_location_assignment PIN_Y24  -to {sseg_out[4][2]}
set_location_assignment PIN_W22  -to {sseg_out[4][3]}
set_location_assignment PIN_W24  -to {sseg_out[4][4]}
set_location_assignment PIN_V23  -to {sseg_out[4][5]}
set_location_assignment PIN_W25  -to {sseg_out[4][6]}
set_location_assignment PIN_V25  -to {sseg_out[5][0]}
set_location_assignment PIN_AA28 -to {sseg_out[5][1]}
set_location_assignment PIN_Y27  -to {sseg_out[5][2]}
set_location_assignment PIN_AB27 -to {sseg_out[5][3]}
set_location_assignment PIN_AB26 -to {sseg_out[5][4]}
set_location_assignment PIN_AA26 -to {sseg_out[5][5]}
set_location_assignment PIN_AA25 -to {sseg_out[5][6]}

set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to {sseg_out[0][0]}
set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to {sseg_out[0][1]}
set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to {sseg_out[0][2]}
set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to {sseg_out[0][3]}
set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to {sseg_out[0][4]}
set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to {sseg_out[0][5]}
set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to {sseg_out[0][6]}
set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to {sseg_out[1][0]}
set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to {sseg_out[1][1]}
set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to {sseg_out[1][2]}
set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to {sseg_out[1][3]}
set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to {sseg_out[1][4]}
set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to {sseg_out[1][5]}
set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to {sseg_out[1][6]}
set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to {sseg_out[2][0]}
set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to {sseg_out[2][1]}
set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to {sseg_out[2][2]}
set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to {sseg_out[2][3]}
set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to {sseg_out[2][4]}
set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to {sseg_out[2][5]}
set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to {sseg_out[2][6]}
set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to {sseg_out[3][0]}
set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to {sseg_out[3][1]}
set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to {sseg_out[3][2]}
set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to {sseg_out[3][3]}
set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to {sseg_out[3][4]}
set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to {sseg_out[3][5]}
set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to {sseg_out[3][6]}
set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to {sseg_out[4][0]}
set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to {sseg_out[4][1]}
set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to {sseg_out[4][2]}
set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to {sseg_out[4][3]}
set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to {sseg_out[4][4]}
set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to {sseg_out[4][5]}
set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to {sseg_out[4][6]}
set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to {sseg_out[5][0]}
set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to {sseg_out[5][1]}
set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to {sseg_out[5][2]}
set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to {sseg_out[5][3]}
set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to {sseg_out[5][4]}
set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to {sseg_out[5][5]}
set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to {sseg_out[5][6]}

