#!/usr/bin/env bash

set -Eeuo pipefail
IFS=$'\n\t'

readonly __SOURCE__=$(basename "$0")
readonly __DIR__=$(cd "$(dirname "$0")" && pwd)

errHandler() {
  local msg="$0"
  (($# > 2)) && msg="${msg}: line $3"
  printf "%s: [ERROR] %s\n" "${msg}" "$2" >&2
  exit $1
}
trap 'errHandler 1 "Exited with status $?" "${LINENO}"' ERR

# File          : run.sh
# Author        : Fabio Scatozza <s315216@studenti.polito.it>
# Date          : 25.07.2025
# Description   : TeX compilation flow

main() {
  command -v pdflatex >/dev/null ||
    errHandler 2 "pdflatex not installed or not in PATH"

  command -v biber >/dev/null ||
    errHandler 2 "biber not installed or not in PATH"

  pushd ${__DIR__} >/dev/null

  # where to carry out compilation
  local work_dir="build"
  local src_tex="main"

  # initialize workspace
  rm -f "${src_tex}.pdf"
  rm -rf "${work_dir}"
  mkdir -p "${work_dir}"

  pdflatex --interaction=nonstopmode --shell-escape --output-directory=${work_dir} "${src_tex}.tex" ||
    errHandler 3 "pdflatex() failed"

  pushd ${work_dir} >/dev/null

  biber --input-directory=${__DIR__} ${src_tex} ||
    errHandler 3 "biber() failed"

  popd >/dev/null

  pdflatex --interaction=nonstopmode --shell-escape --output-directory=${work_dir} "${src_tex}.tex" ||
    errHandler 3 "pdflatex() failed"

  pdflatex --interaction=nonstopmode --shell-escape --output-directory=${work_dir} "${src_tex}.tex" ||
    errHandler 3 "pdflatex() failed"

  mv ${work_dir}/${src_tex}.pdf ${__DIR__}

  popd >/dev/null

  exit 0
}

main "${@}"
