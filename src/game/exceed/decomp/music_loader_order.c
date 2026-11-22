/* Music loader thread ordering adapter: a host adapter around the music loader
 * thread entry point, not a decompilation (issue #9 follow-up,
 * docs/design/20261123-i009-music-loader-race.md,
 * docs/analysis/exceed-xbe-overview.md section 11).
 *
 * All three music start functions (sub_000619C0 at 0x000629C6, sub_0007E140 at
 * 0x0007E1F3, sub_0007E230 at 0x0007E286) create the loader thread
 * (sub_0007DFD0) first and write the "loading" flag MEM8(0x00843550) = 1 only
 * after CreateThread returns. On the single-core Xbox the new same-priority
 * thread does not overtake its creator across those few instructions; on a
 * multi-core host the loader can finish and write 0 first, after which the
 * creator overwrites it with 1 and leaves the title stuck thinking music is
 * still loading.
 *
 * This adapter waits until MEM8(0x00843550) == 1 (capped at 100 ms so a
 * non-standard caller cannot hang) and then forwards directly to the lifted
 * body sub_0007DFD0_gen with guest registers and stack untouched. As a wrapper
 * that calls the lifted body last, it leaves EBP and flags publishing to that
 * body (docs/CODING_STYLE.md). */

#include "recomp_types.h"
#include "reexceed/exceed_host.h"

extern void sub_0007DFD0_gen(void);

enum
{
    kMusicLoadingFlagAddr = 0x00843550u,
    kWaitCapMs = 100u,
    kWaitCapYields = 100000u
};

static void (*s_body)(void) = sub_0007DFD0_gen;

void reexceed_decomp_set_music_loader_body(void (*body)(void))
{
    s_body = body != 0 ? body : sub_0007DFD0_gen;
}

static uint8_t read_loading_flag(void)
{
    return *(const volatile uint8_t*)&MEM8(kMusicLoadingFlagAddr);
}

/* 0x0007DFD0, stdcall thread entry: DWORD (LPVOID path) */
void sub_0007DFD0(void)
{
    if (read_loading_flag() != 1u)
    {
        if (reexceed_host_milliseconds != 0)
        {
            const uint32_t start_ms = reexceed_host_milliseconds();
            while (read_loading_flag() != 1u &&
                   (uint32_t)(reexceed_host_milliseconds() - start_ms) < kWaitCapMs)
            {
                if (reexceed_host_yield != 0)
                {
                    reexceed_host_yield();
                }
            }
        }
        else
        {
            uint32_t yields = 0;
            while (read_loading_flag() != 1u && yields < kWaitCapYields)
            {
                if (reexceed_host_yield != 0)
                {
                    reexceed_host_yield();
                }
                ++yields;
            }
        }
    }
    s_body();
}
