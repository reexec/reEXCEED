#!/usr/bin/env python3
"""Regenerate the lifted C for the Pump It Up: Exceed North America build (gen/exceed_us/).

A thin wrapper over this project's pipeline (lifter.py: XDK identification,
parse, disasm, func_id, abi, HLE and decompiled overrides, lift) that fills
in this title's paths: the XBE under roms/, the recomp configuration in
src/game/exceed/recomp/, the decompiled sources in src/game/exceed/decomp/ and
this project's own xboxrecomp pin (third_party/xboxrecomp). Any of them can
still be given on the command line (--help).

    python scripts/recomp/regenerate.py
    python scripts/recomp/regenerate.py --xbe <path> --variant exceed_us
"""

import os
import pathlib
import sys

import lifter

ROOT = lifter.ROOT
TITLE = ROOT / "src" / "game" / "exceed"
# Linux trees: build/<preset>, or $REEXCEED_BUILD_DIR/<preset> (scripts/build_linux.sh).
LINUX_TREES = [pathlib.Path(base) / f"linux-{arch}-{config}"
               for base in filter(None, [os.environ.get("REEXCEED_BUILD_DIR"), ROOT / "build"])
               for arch in ("x64", "x86") for config in ("release", "debug")]

if __name__ == "__main__":
    sys.exit(lifter.run(
        xbe=lifter.find_case_insensitive(ROOT / "roms", "default.xbe"),
        variant="exceed_us",
        title_dir=TITLE / "recomp",
        override_dirs=[TITLE / "decomp"],
        gen_root=ROOT / "gen",
        xboxrecomp=ROOT / "third_party" / "xboxrecomp",
        symbol_tool_candidates=[
            ROOT / "build" / "win32_x86" / "third_party" / "rexbox" / "tools" / "Release" / "rexbox_xdk_symbols.exe",
            ROOT / "build" / "win32_x86" / "third_party" / "rexbox" / "tools" / "Debug" / "rexbox_xdk_symbols.exe",
            ROOT / "build" / "third_party" / "rexbox" / "tools" / "rexbox_xdk_symbols",
        ] + [tree / "third_party" / "rexbox" / "tools" / "rexbox_xdk_symbols" for tree in LINUX_TREES],
        root=ROOT,
    ))
