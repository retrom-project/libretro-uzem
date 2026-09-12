#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "${BASH_SOURCE[0]}")/../.."
make -f Makefile.libretro clean
make -f Makefile.libretro -j4
mapfile -t objects < <(find . -name '*.o' -not -path './build/*' -print)
mkdir -p build/native-test
c++ -I. -Ilibretro-common/include -D__LIBRETRO__ -DNOGDB \
  .github/rpg-runtime/test-state.cpp "${objects[@]}" -o build/native-test/test-state
build/native-test/test-state
if [[ $# == 1 ]]; then
  python3 .github/rpg-runtime/test-game.py ./uzem_libretro.so "$1"
fi
