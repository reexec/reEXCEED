#!/usr/bin/env bash
# Builds and tests one Linux preset (CMakePresets.json).
#
#   scripts/build_linux.sh [x64|x86] [debug|release] [--no-test]
#
# x86 is a -m32 build: it needs gcc-multilib/g++-multilib and the 32-bit
# development libraries SDL3 compiles against (docs/guides/linux-setup-and-build.md).
# The tree goes to build/linux-<arch>-<config>, or under $REEXCEED_BUILD_DIR when it
# is set (on WSL2 a tree on the Linux file system builds much faster than one
# under /mnt).
set -euo pipefail

arch="${1:-x64}"
config="${2:-debug}"
run_tests=1
for argument in "$@"; do
    if [ "$argument" = "--no-test" ]; then
        run_tests=0
    fi
done
case "$arch" in x64|x86) ;; *) echo "unknown architecture: $arch (x64 or x86)" >&2; exit 2 ;; esac
case "$config" in debug|release) ;; *) echo "unknown configuration: $config (debug or release)" >&2; exit 2 ;; esac

preset="linux-$arch-$config"
root="$(cd "$(dirname "$0")/.." && pwd)"
cd "$root"
if [ -n "${REEXCEED_BUILD_DIR:-}" ]; then
    tree="$REEXCEED_BUILD_DIR/$preset"
    cmake --preset "$preset" -B "$tree"
    cmake --build "$tree" --parallel
    if [ "$run_tests" = 1 ]; then
        ctest --test-dir "$tree" --output-on-failure
    fi
else
    cmake --preset "$preset"
    cmake --build --preset "$preset" --parallel
    if [ "$run_tests" = 1 ]; then
        ctest --preset "$preset"
    fi
fi
