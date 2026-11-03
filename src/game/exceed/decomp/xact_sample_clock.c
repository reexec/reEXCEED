/* Host adapter for XACT's time base (issue #4, docs/analysis/exceed-xbe-overview.md
 * section 11, rexbox issue #55).
 *
 * Not a straight decompilation: 0x0016F12A is rewritten with the same
 * results, except that the APU's global sample counter, which the lifted body
 * reads from its MMIO register at 0xFE80200C (outside guest memory under the
 * HLE), comes from the host hook reexceed_host_apu_sample_count. The original
 * raises the IRQL around the read (0x0016689E, KfLowerIrql through the thunk
 * at [0x0018F978]); with no interrupt racing a host read, the adapter does
 * not. */

#include "recomp_types.h"
#include "reexceed/exceed_host.h"

/* 0x0016F12A, stdcall with an implicit esi: void (XACT engine* esi,
 * REFERENCE_TIME* pTime)
 *
 * The engine keeps the last counter at +0x144 and the 64-bit sample total at
 * +0x148. The delta since the last call is added: forward as is, across a
 * 32-bit wrap (counter below 100000, last above 0x40000000) with the wrap,
 * otherwise (a counter that went back) as 0. *pTime is then
 * total x 10000 / 48000 x 1000 (100 ns units), with the original's 64-bit
 * truncating multiply (_allmul) and unsigned divide (_aulldiv); edx:eax holds
 * the same value on return. */
void sub_0016F12A(void)
{
    const uint32_t engine = g_esi;
    const uint32_t time_out = MEM32(g_esp + 4);
    const uint32_t counter = reexceed_host_apu_sample_count != 0 ? reexceed_host_apu_sample_count() : 0;
    const uint32_t last = MEM32(engine + 0x144);
    uint64_t delta = 0;
    if (counter >= last)
    {
        delta = counter - last;
    }
    else if (counter < 0x186A0u && last > 0x40000000u)
    {
        delta = (uint64_t)counter + 0x100000000ull - last;
    }
    MEM32(engine + 0x144) = counter;
    uint64_t total = ((uint64_t)MEM32(engine + 0x14C) << 32) | MEM32(engine + 0x148);
    total += delta;
    MEM32(engine + 0x148) = (uint32_t)total;
    MEM32(engine + 0x14C) = (uint32_t)(total >> 32);
    const uint64_t time = total * 10000u / 48000u * 1000u;
    MEM32(time_out) = (uint32_t)time;
    MEM32(time_out + 4) = (uint32_t)(time >> 32);
    g_eax = (uint32_t)time;
    g_edx = (uint32_t)(time >> 32);
    g_ecx = time_out; /* the original leaves pTime in ecx */
    g_esp += 8; /* ret 4 */
}
