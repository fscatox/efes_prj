# File          : gen_prj.tcl
# Author        : Fabio Scatozza <s315216@studenti.polito.it>
# Date          : 15.12.2024
# Description   : Generate Quartus Prime project for DE1-SoC board

package require cmdline

# recursive glob
proc rglob {dir pattern} {

  # normalize separators adding one at the end
  set dir [string trimright [file join [file normalize $dir] { }]]
  set files {}

  foreach file [glob -nocomplain -type f -directory $dir $pattern] {
    lappend files $file
  }

  foreach sub_dir [glob -nocomplain -type d -directory $dir *] {
    foreach sub_file [rglob $sub_dir $pattern] {
      lappend files $sub_file
    }
  }

  return $files
}

set optfmt {
  { "proj.arg"    ""              "Project name (default: top module)" }
  { "rev.arg"     "origin"        "Revision name" }
  { "top.arg"     ""              "Top level module" }
  { "rtldir.arg"  ""              "RTL design sources" }
  { "sdc.arg"     ""              "Timing constraints" }
  { "pins.arg"    ""              "Pin assignments" }
  { "outdir.arg"  "output_files"  "Generated outputs directory" }
}
array set qargs [::cmdline::getoptions ::quartus(args) $optfmt]

# create and open project
project_new -revision $qargs(rev) [expr {$qargs(proj) != {} ? $qargs(proj) : $qargs(top)}]

# platform selection
set_global_assignment -name FAMILY "Cyclone V"
set_global_assignment -name DEVICE 5CSEMA5F31C6
set_global_assignment -name BOARD "DE1-SoC Board"

# tool configuration
set_global_assignment -name NUM_PARALLEL_PROCESSORS ALL
set_global_assignment -name PROJECT_OUTPUT_DIRECTORY $qargs(outdir)

# import sources
foreach hdl { {vhd VHDL} {\{v,sv\} SYSTEMVERILOG} } {
  lassign $hdl ext asgmt

  foreach src [rglob $qargs(rtldir) *.$ext] {
    set_global_assignment -name ${asgmt}_FILE $src
  }
}
set_global_assignment -name SDC_FILE $qargs(sdc)

# project configuration
set_global_assignment -name TOP_LEVEL_ENTITY $qargs(top)

# import pin assignments
source [file normalize $qargs(pins)]

# export assignments and exit
project_close

