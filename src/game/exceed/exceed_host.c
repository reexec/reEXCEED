/* The host hooks of include/reexceed/exceed_host.h: defined here, outside
 * decomp/, because the decompiled sources are compiled only next to the
 * generated code, while the application sets the hooks in every build. */
#include "reexceed/exceed_host.h"

float (*reexceed_host_horizontal_view_scale)(void) = 0;
uint32_t (*reexceed_host_apu_sample_count)(void) = 0;

static volatile int s_unlock_all = 0;
static volatile int s_autoplay = 0;
static volatile int s_full_size_picture = 1;

void reexceed_set_unlock_all(int enabled)
{
    s_unlock_all = enabled != 0 ? 1 : 0;
#if defined(REEXCEED_HAS_RECOMP)
    reexceed_decomp_sync_unlock_all();
#endif
}

int reexceed_host_unlock_all(void)
{
    return s_unlock_all;
}

void reexceed_set_autoplay(int enabled)
{
    s_autoplay = enabled != 0 ? 1 : 0;
#if defined(REEXCEED_HAS_RECOMP)
    reexceed_decomp_sync_autoplay();
#endif
}

int reexceed_host_autoplay(void)
{
    return s_autoplay;
}

void reexceed_set_full_size_picture(int enabled)
{
    s_full_size_picture = enabled != 0 ? 1 : 0;
#if defined(REEXCEED_HAS_RECOMP)
    reexceed_decomp_sync_full_size_picture();
#endif
}

int reexceed_host_full_size_picture(void)
{
    return s_full_size_picture;
}
