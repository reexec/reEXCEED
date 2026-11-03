# Exceed XBE Structure and Design (English)

Accumulates the structure confirmed by analysing the original XBE. Detailed evidence and confirmation status live in [`docs/analysis/`](analysis/README.md); only conclusions that affect the design are collected here. Korean version: [XBE_DESIGN.ko.md](XBE_DESIGN.ko.md).

## 1. Target build

* The primary target is the North America build `default.xbe` (region 1, title id `41440002`, MD5 `575f861e…`, XDK 5849). → [exceed-xbe-overview §1](analysis/exceed-xbe-overview.md)
* The disc's three other XBEs (`UPDATE`, `DOWNLOADER`, `DASHUPDATE`) are Xbox Live and dashboard tools and not targets.
* Another regional build, if found, is recognized only and assumed to share no addresses (**inferred**).

## 2. Memory layout

The diagram in the Korean version applies: headers at `0x00010000`, `.text` at `0x00011000`, the libraries (`D3DX`, `DSOUND`, `WMADEC`, `XONLINE`, `XNET`, `XMV`, `XACTENG`, `XGRPH`, `D3D`, `XPP`) from `0x000DC720`, `.rdata` at `0x0018F8C0` (kernel thunks), `.data` at `0x001BDF40` (about 8.9 MiB, mostly BSS), `DOLBY` at `0x00A98440`, then `.data1`, `XON_RD`, `$$XTIMAGE`, `$$XSIMAGE` and `.XTLID`.

* The image ends at base + `0xA94D20`, inside the 64 MiB of guest RAM.
* The kernel thunk table is at `.rdata`'s first address `0x0018F8C0`, with 146 slots.
* Physical address window: whether the title uses `| 0x80000000` is not known yet; the app turns rexbox's physical window on as reCT3 does (`GuestMemory::Options::physical_window`, rexbox design i008).

## 3. Conclusions for the HLE design

* XDK 5849: whether rexbox's D3D8 and DSOUND HLE need the 5849 layout is checked after the lift (**inferred**).
* WMA (`WMADEC`), XMV and XACT (`XACTENG`) run as lifted CPU code; their hardware boundaries are DSOUND (sound), D3D8 (movie frames) and the kernel (files).
* 42 of the 146 kernel imports have no HLE export (analysis §4), mostly the I/O manager, cryptography and networking; they are implemented in rexbox when the boot reaches them.

## 4. Start sequence and kernel structures

* The entry point is `0x0008673A`; the start sequence is established by the lift and the first run (**unresolved**).
