#!/usr/bin/env python3
"""Self-check for lifter_patches.py (issue #6).

Run: py -3 scripts/recomp/test_lifter_patches.py

1. Regression guard: everything the patches do not target (the 32-bit
   one-operand forms, the two- and three-operand imul, div and idiv) lifts to
   exactly the text upstream v0.16.0 emits.
2. The narrow one-operand products, compiled, match the x86 rules for a set
   of register values: `imul/mul r/m8` writes AX only, `imul/mul r/m16` writes
   AX and DX only. Needs a C compiler (cl, clang or gcc on PATH, or MSVC found
   through vswhere); without one this part is reported as skipped.
"""

import os
import pathlib
import random
import shutil
import subprocess
import sys
import tempfile

HERE = pathlib.Path(__file__).resolve().parent
XBOXRECOMP = HERE.parents[1] / "third_party" / "xboxrecomp"
sys.path.insert(0, str(XBOXRECOMP))
sys.path.insert(0, str(HERE))

from tools.recomp import lifter as lifter_module  # noqa: E402
import lifter_patches  # noqa: E402


class _Op:
    def __init__(self, text, type="reg", reg=None, size=4, mem_size=None, imm=0):
        self.text = text
        self.type = type
        self.reg = reg
        self.size = size
        self.mem_size = mem_size
        self.imm = imm


class _Insn:
    def __init__(self, mnemonic, operands):
        self.mnemonic = mnemonic
        self.operands = operands


def _reg(name):
    return _Op(name, reg=name)


def _mem(size):
    op = _Op("byte ptr [ecx]" if size == 1 else "word ptr [ecx]" if size == 2 else "dword ptr [ecx]",
             type="mem", size=size, mem_size=size)
    op.mem_base, op.mem_index, op.mem_scale, op.mem_disp = "ecx", None, 1, 0
    return op


def _lift(mnemonic, ops):
    lifter = lifter_module.Lifter()
    lifter.needs_cf = True
    return lifter.lift_instruction(_Insn(mnemonic, ops))


# Forms the patches must leave as upstream emits them.
UNCHANGED = [
    ("imul", [_reg("ecx")]),
    ("imul", [_mem(4)]),
    ("mul", [_reg("ebx")]),
    ("mul", [_mem(4)]),
    ("imul", [_reg("eax"), _reg("ecx")]),
    ("imul", [_reg("cx"), _reg("dx")]),
    ("imul", [_reg("eax"), _reg("ecx"), _Op("0x34", type="imm", imm=0x34)]),
    ("div", [_reg("ecx")]),
    ("div", [_reg("cl")]),
    ("idiv", [_reg("ecx")]),
    ("idiv", [_mem(1)]),
]

# Narrow forms: (mnemonic, operand, source register written by the harness).
NARROW = [
    ("imul", "bl", "ebx", 8, True),
    ("imul", "cl", "ecx", 8, True),
    ("imul", "dh", "edx", 8, True),
    ("mul", "cl", "ecx", 8, False),
    ("mul", "bh", "ebx", 8, False),
    ("imul", "cx", "ecx", 16, True),
    ("mul", "bx", "ebx", 16, False),
]


def check_unchanged():
    before = [_lift(m, ops) for m, ops in UNCHANGED]
    lifter_patches.apply()
    after = [_lift(m, ops) for m, ops in UNCHANGED]
    failures = 0
    for (m, ops), b, a in zip(UNCHANGED, before, after):
        if a != b:
            failures += 1
            print(f"FAIL unchanged {m} {', '.join(o.text for o in ops)}:\n  upstream {b}\n  patched  {a}")
    print(f"unchanged forms: {len(UNCHANGED) - failures}/{len(UNCHANGED)} identical to upstream")
    return failures


def reference(mnemonic, operand, eax, ebx, ecx, edx, width, signed):
    """x86's eax and edx after a one-operand product."""
    regs = {"eax": eax, "ebx": ebx, "ecx": ecx, "edx": edx}
    full = {"bl": ("ebx", 0), "bh": ("ebx", 8), "cl": ("ecx", 0), "dh": ("edx", 8), "cx": ("ecx", 0),
            "bx": ("ebx", 0)}[operand]
    mask = (1 << width) - 1
    src = (regs[full[0]] >> full[1]) & mask
    acc = eax & mask

    def sx(v):
        return v - (1 << width) if signed and v & (1 << (width - 1)) else v

    product = (sx(acc) * sx(src)) & ((1 << (2 * width)) - 1)
    if width == 8:
        return (eax & 0xFFFF0000) | product, edx
    return (eax & 0xFFFF0000) | (product & 0xFFFF), (edx & 0xFFFF0000) | (product >> 16)


def find_compiler():
    """A command prefix that compiles C, and whether it is MSVC."""
    for name in ("cl", "clang", "gcc", "cc"):
        if shutil.which(name):
            return [name], name == "cl"
    vswhere = pathlib.Path(os.environ.get("ProgramFiles(x86)", r"C:\Program Files (x86)")) / \
        "Microsoft Visual Studio" / "Installer" / "vswhere.exe"
    if vswhere.exists():
        path = subprocess.run([str(vswhere), "-latest", "-property", "installationPath"],
                              capture_output=True, text=True).stdout.strip()
        vcvars = pathlib.Path(path) / "VC" / "Auxiliary" / "Build" / "vcvars32.bat"
        if vcvars.exists():
            return ["cmd", "/c", str(vcvars), ">nul", "&&", "cl"], True
    return None, False


def check_narrow():
    command, msvc = find_compiler()
    if command is None:
        print("narrow forms: SKIPPED (no C compiler)")
        return 0
    rng = random.Random(6)
    values = [0, 1, 0x7F, 0x80, 0xFF, 0x7FFF, 0x8000, 0xFFFF, 0xFFFFFFFF, 0xA5A55A5A, 0x12345678]
    vectors = [(a, b, c, d) for a in values for b in values[:6] for c in values[6:] for d in (0xA5A55A5A,)]
    vectors += [tuple(rng.getrandbits(32) for _ in range(4)) for _ in range(400)]

    lifter_patches.apply()
    cases = []
    body = ["#include <stdint.h>", "#include <stdio.h>",
            "#define LO8(r) ((uint32_t)((r) & 0xFFu))",
            "#define HI8(r) ((uint32_t)(((r) >> 8) & 0xFFu))",
            "#define LO16(r) ((uint32_t)((r) & 0xFFFFu))",
            "#define SET_LO16(r, v) ((r) = ((r) & 0xFFFF0000u) | ((uint32_t)(uint16_t)(v)))",
            "static uint32_t eax, ebx, ecx, edx; static int _cf;"]
    for index, (m, operand, _, width, signed) in enumerate(NARROW):
        lines = _lift(m, [_reg(operand)])
        if any("edx =" in line or "eax =" in line for line in lines):
            print(f"FAIL {m} {operand}: writes a whole register: {lines}")
            return 1
        body.append(f"static void case{index}(void) {{ {' '.join(lines)} }}")
        cases.append(index)
    body.append("int main(void) { unsigned a, b, c, d; int k;")
    body.append("  while (scanf(\"%d %x %x %x %x\", &k, &a, &b, &c, &d) == 5) {")
    body.append("    eax = a; ebx = b; ecx = c; edx = d;")
    body.append("    switch (k) {" + " ".join(f"case {i}: case{i}(); break;" for i in cases) + "}")
    body.append("    printf(\"%08X %08X\\n\", eax, edx); }")
    body.append("  return 0; }")

    failures = 0
    with tempfile.TemporaryDirectory() as tmp:
        source = pathlib.Path(tmp) / "narrow.c"
        exe = pathlib.Path(tmp) / "narrow.exe"
        source.write_text("\n".join(body) + "\n")
        build = command + ([str(source), "/nologo", "/O2", "/Fe:" + str(exe), "/Fo:" + str(tmp) + "\\"]
                           if msvc else [str(source), "-O2", "-o", str(exe)])
        result = subprocess.run(build, capture_output=True, text=True, cwd=tmp)
        if result.returncode != 0:
            print("FAIL compile:", result.stdout, result.stderr)
            return 1
        stdin = []
        expected = []
        for index, (m, operand, _, width, signed) in enumerate(NARROW):
            for a, b, c, d in vectors:
                stdin.append(f"{index} {a:x} {b:x} {c:x} {d:x}")
                expected.append((index, a, b, c, d, reference(m, operand, a, b, c, d, width, signed)))
        out = subprocess.run([str(exe)], input="\n".join(stdin) + "\n", capture_output=True, text=True).stdout.split()
        for n, (index, a, b, c, d, (want_eax, want_edx)) in enumerate(expected):
            got_eax, got_edx = int(out[2 * n], 16), int(out[2 * n + 1], 16)
            if (got_eax, got_edx) != (want_eax, want_edx):
                failures += 1
                if failures <= 10:
                    m, operand = NARROW[index][:2]
                    print(f"FAIL {m} {operand} eax={a:08X} ebx={b:08X} ecx={c:08X} edx={d:08X}: "
                          f"got {got_eax:08X} {got_edx:08X}, x86 {want_eax:08X} {want_edx:08X}")
    print(f"narrow forms: {len(expected) - failures}/{len(expected)} vectors match x86 "
          f"({len(NARROW)} forms)")
    return failures


def main():
    failures = check_unchanged() + check_narrow()
    print("OK" if failures == 0 else f"{failures} failure(s)")
    return 1 if failures else 0


if __name__ == "__main__":
    sys.exit(main())
