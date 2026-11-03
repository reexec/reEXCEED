/* Host adapter for XAPI start-up's code-selector limit (issue #4,
 * docs/design/20261103-i004-first-run.md, docs/analysis/exceed-xbe-overview.md
 * section 7).
 *
 * Not a decompilation: the lifted body is replaced by one that does nothing.
 * At start-up (mainXapiStartup 0x000866C6 -> 0x00087E76, which tail-jumps
 * here) XDK 5849's XAPI reads the kernel's PE header at 0x80010000
 * (e_lfanew at 0x8001003C), finds the end of the kernel's last section and
 * builds a code-segment descriptor (0x00C09B00 | limit) covering only that
 * much; 0x0008A804 then swaps it into GDT entry 1 (selector 8) with sgdt and
 * reloads CS with a far jump, and stores the old descriptor at
 * [0x00A93978]/[0x00A9397C] for 0x0008A7E0 to put back before a reboot.
 *
 * Under the HLE there is no kernel image and no GDT: rexbox's physical window
 * shows the title's own XBE header at 0x80010000, so the lifted body reads a
 * garbage section table and faults outside guest RAM. Doing nothing leaves
 * the saved descriptor zero, which is exactly what tells the restore path to
 * skip (it tests [0x00A93978] | [0x00A9397C] for zero). */

#include "recomp_types.h"

/* 0x00087E12, cdecl: void (void)
 *
 * Restricts the kernel code selector's limit to the kernel image; here a
 * plain return. */
void sub_00087E12(void)
{
    g_esp += 4; /* ret */
}
