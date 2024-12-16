# File          : build.tcl
# Author        : Fabio Scatozza <s315216@studenti.polito.it>
# Date          : 16.12.2024
# Description   : Quartus Prime compilation flow

package require cmdline
load_package flow

set optfmt {
  { "proj.arg" ""       "Project name" }
  { "rev.arg"  "origin" "Revision name" }
}
array set qargs [::cmdline::getoptions ::quartus(args) $optfmt]

# open project
project_open -revision $qargs(rev) $qargs(proj)

# run default flow
execute_flow -compile

project_close
