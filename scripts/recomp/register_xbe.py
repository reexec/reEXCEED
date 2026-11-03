#!/usr/bin/env python3
"""Register the user's Pump It Up: Exceed XBE as the variant the project is lifted from.

Reads the XBE's identity (MD5 and the certificate fields a title profile
identifies a build by), writes the MD5 into src/game/exceed/recomp/variants.json
for the variant, and prints the C++ initializer to paste into
src/game/exceed/exceed_profile.cpp (kVariants) and the analysis table for
docs/analysis/exceed-xbe-overview.md. Nothing from the XBE's code is written
anywhere; the certificate fields are the build's identity only.

    python scripts/recomp/register_xbe.py                      # roms/default.xbe as exceed_us
    python scripts/recomp/register_xbe.py --xbe <path> --variant exceed_us --label "North America"
    python scripts/recomp/register_xbe.py --dry-run            # print only
"""

import argparse
import json
import pathlib
import sys

ROOT = pathlib.Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "third_party" / "rexbox" / "scripts" / "recomp"))
from xbe_variant import xbe_identity  # noqa: E402  (rexbox's)
sys.path.insert(0, str(ROOT / "scripts" / "recomp"))
from lifter import find_case_insensitive  # noqa: E402

VARIANTS = ROOT / "src" / "game" / "exceed" / "recomp" / "variants.json"


def main():
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--xbe", type=pathlib.Path, default=find_case_insensitive(ROOT / "roms", "default.xbe"))
    parser.add_argument("--variant", default="exceed_us")
    parser.add_argument("--label", default=None, help="the variant's label (default: keep the file's)")
    parser.add_argument("--dry-run", action="store_true", help="print the identity, write nothing")
    args = parser.parse_args()

    if not args.xbe.is_file():
        sys.exit(f"no XBE at {args.xbe} -- put your own dump under roms/ (docs/guides/)")
    identity = xbe_identity(args.xbe)
    data = json.loads(VARIANTS.read_text(encoding="utf-8"))
    variant = data["variants"].setdefault(args.variant, {"label": args.variant, "md5": None})
    label = args.label or variant.get("label", args.variant)

    print(f"{args.xbe}")
    for key, value in identity.items():
        print(f"  {key:20s} {value}")
    print()
    print("exceed_profile.cpp kVariants entry:")
    print(f'    {{"{label}", {identity["title_id"]}, {identity["certificate_time"]}, '
          f'{identity["certificate_version"]}, {identity["image_time"]}}},')
    print()
    print("docs/analysis/exceed-xbe-overview.md row:")
    print(f'| {label} | {identity["region"]} | `{identity["md5"]}` | {identity["size"]:,} | '
          f'`{identity["title_id"][2:]}` | `{identity["certificate_time"][2:]}` | `{identity["image_time"][2:]}` |')
    if args.dry_run:
        return 0
    previous = variant.get("md5")
    variant["label"] = label
    variant["md5"] = identity["md5"]
    VARIANTS.write_text(json.dumps(data, indent=2, ensure_ascii=False) + "\n", encoding="utf-8", newline="\n")
    print()
    print(f"wrote {VARIANTS.relative_to(ROOT)}: {args.variant} md5 {previous} -> {identity['md5']}")
    print("next: paste the kVariants entry into src/game/exceed/exceed_profile.cpp, then run scripts/recomp/regenerate.py")
    return 0


if __name__ == "__main__":
    sys.exit(main())
