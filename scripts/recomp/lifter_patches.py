#!/usr/bin/env python3
"""Temporary fixes to the pinned upstream lifter, applied when this project lifts.

usage: lifter_patches.py <module> [args...]
    Runs `python -m <module> args...` (from the xboxrecomp checkout, the
    current directory) with the fixes below applied first. lifter.py's `lift`
    step runs tools.recomp this way; the other steps run as before.

The checkout stays unmodified (lifter.py checks it): the fixes replace methods
of tools.recomp.lifter.Lifter in this process only. Each one names the upstream
change that carries it, and is removed when an upstream release we pin has it.

There are no fixes at present. When a lifter defect shows, its fix goes in
apply() below (a method of tools.recomp.lifter.Lifter replaced in this
process), with the issue, the upstream change that carries it, and a check in
test_lifter_patches.py.

Removed fixes
    narrow one-operand products (issue #6): upstream sp00nznet/xboxrecomp#277,
    in v0.17.0, lifts 8- and 16-bit `mul`/`imul` at their own width; removed
    when v0.18.0 was pinned (issue #18). test_lifter_patches.py keeps checking
    those forms against x86.
"""

import os
import runpy
import sys


def apply():
    """Replaces the Lifter methods the fixes above change (none at present)."""
    from tools.recomp import lifter as lifter_module

    lifter_class = lifter_module.Lifter
    if getattr(lifter_class, "_reexceed_patched", False):
        return
    lifter_class._reexceed_patched = True


def main():
    if len(sys.argv) < 2:
        sys.exit(__doc__)
    module = sys.argv[1]
    # As `python -m` would: the current directory (the checkout) first.
    sys.path.insert(0, os.getcwd())
    apply()
    sys.argv = [module] + sys.argv[2:]
    runpy.run_module(module, run_name="__main__", alter_sys=True)


if __name__ == "__main__":
    main()
