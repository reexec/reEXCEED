/* Full-size picture: a host adapter around the title's 2D setup, not a
 * decompilation (issue #14, docs/design/20261107-i014-full-size-picture.md,
 * docs/analysis/exceed-xbe-overview.md section 12).
 *
 * Graphics init (sub_000765E0) sets the virtual screen size to the back
 * buffer size times 1.2 (1.1 on the HDTV AV pack) to make room for TV
 * overscan. The 2D projection (sub_00075F10) and the 3D field of view
 * (sub_00075FE0) are built from it, so the whole picture shrinks to 1/1.2.
 * With reexceed_host_full_size_picture() set, the virtual size is the back
 * buffer size. Init calls sub_00075F10 right after writing the size, so the
 * override is in place before any 2D or 3D setup. */

#include "recomp_types.h"
#include "reexceed/exceed_host.h"

extern void sub_00075F10_gen(void);

enum
{
    kBackBufferWidthAddr = 0x00842334u,   /* int, written by sub_000765E0 */
    kBackBufferHeightAddr = 0x00842398u,  /* int */
    kAvPackAddr = 0x00842338u,            /* HalBootSMCVideoMode at init */
    kVirtualWidthAddr = 0x00842358u,      /* float */
    kVirtualHeightAddr = 0x00842354u,     /* float */
    kScaleHdtvAddr = 0x001BD3BCu,         /* 1.1f */
    kScaleOtherAddr = 0x001BD46Cu,        /* 1.2f */
    kAvPackHdtv = 1u
};

static int s_guest_ready = 0;
static int s_overridden = 0;

static void sync_virtual_size(void)
{
    const int32_t width = (int32_t)MEM32(kBackBufferWidthAddr);
    const int32_t height = (int32_t)MEM32(kBackBufferHeightAddr);
    if (width == 0 || height == 0)
    {
        return;  /* graphics init has not run yet */
    }
    if (reexceed_host_full_size_picture() != 0)
    {
        const float full_width = (float)width;
        const float full_height = (float)height;
        MEMF(kVirtualWidthAddr) = full_width;
        MEMF(kVirtualHeightAddr) = full_height;
        s_overridden = 1;
    }
    else if (s_overridden)
    {
        /* The original values, by sub_000765E0's formula (cvtsi2ss, mulss). */
        const float scale = MEM32(kAvPackAddr) == kAvPackHdtv ? MEMF(kScaleHdtvAddr) : MEMF(kScaleOtherAddr);
        const float scaled_width = (float)width * scale;
        const float scaled_height = (float)height * scale;
        MEMF(kVirtualWidthAddr) = scaled_width;
        MEMF(kVirtualHeightAddr) = scaled_height;
        s_overridden = 0;
    }
}

void reexceed_decomp_sync_full_size_picture(void)
{
    if (s_guest_ready)
    {
        sync_virtual_size();
    }
}

/* 2D setup: View identity, Projection OrthoLH(virtual width, virtual height,
 * -500, 500). */
void sub_00075F10(void)
{
    s_guest_ready = 1;
    sync_virtual_size();
    sub_00075F10_gen();
}
