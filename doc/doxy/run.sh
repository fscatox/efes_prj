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
# Description   : Doxygen compilation

main() {
  command -v doxygen >/dev/null ||
    errHandler 2 "doxygen not installed or not in PATH"

  pushd ${__DIR__} >/dev/null

  doxygen Doxyfile ||
    errHandler 3 "doxygen() failed"

  ln -s html/index.html .

  popd >/dev/null

  exit 0
}

main "${@}"
