#!/usr/bin/env python3
"""Temporary fixes to the pinned upstream lifter, applied when this project lifts.

usage: lifter_patches.py <module> [args...]
    Runs `python -m <module> args...` (from the xboxrecomp checkout, the
    current directory) with the fixes below applied first. lifter.py's `lift`
    step runs tools.recomp this way; the other steps run as before.

The checkout stays unmodified (lifter.py checks it): the fixes replace methods
of tools.recomp.lifter.Lifter in this process only. Each one names the upstream
change that carries it, and is removed when an upstream release we pin has it.

narrow one-operand products (issue #6; upstream sp00nznet/xboxrecomp#277)
    v0.16.0 lifts every one-operand `mul`/`imul` as the 32-bit product
    edx:eax = eax * src. For an 8-bit operand the CPU writes AX only, and for a
    16-bit operand DX:AX, so the lifted code clobbered EDX (and the upper half
    of EAX). Exceed's libpng keeps a PNG's height in EDX across `imul bl` in
    png_handle_IHDR, so every PNG got height 0. The fix lifts the 8- and 16-bit
    forms at their own width, as #277 does, and leaves the 32-bit form exactly
    as v0.16.0 emits it. Flags are not touched: the 8- and 16-bit forms keep
    v0.16.0's flag tracking (#277 also changes how CF/OF after a product are
    read; that part is not taken).
"""

import os
import runpy
import sys

# The width (in bits) of the operand: memory operands know their size,
# registers imply it by name. Immediates do not occur in one-operand products.
_NARROW_WIDTHS = (8, 16)


def _narrow_product(lifter_module, ops, signed):
    """C for a one-operand 8- or 16-bit product, or None for any other width."""
    width = (lifter_module._operand_width(ops[0]) or 4) * 8
    if width not in _NARROW_WIDTHS:
        return None
    src = lifter_module._fmt_operand_read(ops[0])
    narrow = f"{'int' if signed else 'uint'}{width}_t"
    wide = "int32_t" if signed else "uint32_t"
    if width == 8:
        # AX = AL * r/m8; the rest of EAX and all of EDX are left alone.
        write = "SET_LO16(eax, _r);"
    else:
        # DX:AX = AX * r/m16; the upper halves of EAX and EDX are left alone.
        write = "SET_LO16(eax, _r); SET_LO16(edx, (uint32_t)_r >> 16);"
    return [f"{{ {wide} _r = ({wide})({narrow})eax * ({wide})({narrow})({src});",
            f"  {write} }}"]


def apply():
    """Replaces the Lifter methods the fixes above change."""
    from tools.recomp import lifter as lifter_module

    lifter_class = lifter_module.Lifter
    if getattr(lifter_class, "_reexceed_patched", False):
        return
    upstream_imul = lifter_class._lift_imul
    upstream_muldiv = lifter_class._lift_muldiv

    def _lift_imul(self, insn, ops):
        if len(ops) == 1:
            narrow = _narrow_product(lifter_module, ops, signed=True)
            if narrow is not None:
                return narrow
        return upstream_imul(self, insn, ops)

    def _lift_muldiv(self, insn, ops, m):
        if m == "mul" and len(ops) == 1:
            narrow = _narrow_product(lifter_module, ops, signed=False)
            if narrow is not None:
                return narrow
        return upstream_muldiv(self, insn, ops, m)

    lifter_class._lift_imul = _lift_imul
    lifter_class._lift_muldiv = _lift_muldiv
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
