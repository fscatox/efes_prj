#!/usr/bin/env bash

set -Eeuo pipefail
IFS=$'\n\t'

readonly __SOURCE__=$(basename "$0")
readonly __DIR__=$(cd "$(dirname "$0")" && pwd)

errHandler() {
  local msg="$0"
  (( $# > 2 )) && msg="${msg}: line $3"
  printf "%s: [ERROR] %s\n" "${msg}" "$2" >&2
  exit $1
}
trap 'errHandler 1 "Exited with status $?" "${LINENO}"' ERR

# File          : run.sh
# Author        : Fabio Scatozza <s315216@studenti.polito.it>
# Date          : 15.12.2024
# Description   : Runs Tcl-based quartus flow

main() {
  command -v quartus_sh > /dev/null || \
    errHandler 2 "Quartus Prime not installed or not in PATH"

  # where to find Tcl scripts for design automation
  # (including pin assignments, timing constraints, ...)
  local eda_dir="${__DIR__}/eda"

  # where to find HDL sources for synthesis
  local rtl_dir="${__DIR__}/hdl/rtl"

  # where to carry out quartus flow
  local work_dir="${__DIR__}/quartus"
  local out_dir_rel="output_files"

  # top module
  local top_module="efes_prj"

  # initialize workspace
  rm -rf "${work_dir}"
  mkdir -p "${work_dir}"

  pushd ${work_dir} > /dev/null

  # generate quartus project
  quartus_sh -t "${eda_dir}/gen_prj.tcl" \
             -top "${top_module}" \
             -rtldir "${rtl_dir}" \
             -pins "${eda_dir}/pin_assignments.tcl" \
             -sdc "${eda_dir}/${top_module}.sdc" \
             -outdir "${out_dir_rel}"

  # run quartus flow
  quartus_sh -t "${eda_dir}/build.tcl" \
             -proj "${top_module}"

  popd > /dev/null

  printf "\nOutputs written to %s\n" "${work_dir}/${out_dir_rel}"
  exit 0

}

main "${@}"

