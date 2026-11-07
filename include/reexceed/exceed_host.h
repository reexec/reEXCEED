// reexceed: what the title's hand-written adapters ask of the host. Plain C so
// the decompiled sources (C) can include it; the application fills the hooks
// in, and reexceed_game links without the HLE (the decomp check tool), where
// each hook's null means its 4:3, no-host answer.
#ifndef REEXCEED_EXCEED_HOST_H
#define REEXCEED_EXCEED_HOST_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// How much wider the picture's horizontal view is than the title's 4:3 one:
// the widened back buffer's width over 640, or 1
// (reJSRF design i097). Null: 1.
extern float (*reexceed_host_horizontal_view_scale)(void);

// The APU's global sample counter (0xFE80200C on the console, 48 kHz) that
// XACT reads as its time base (decomp/xact_sample_clock.c, rexbox issue #55).
// Null: 0, a clock that never advances.
extern uint32_t (*reexceed_host_apu_sample_count)(void);

// Non-destructive temporary unlock of all songs, charts, speed/note modifiers,
// and sub-modes (issue #13, docs/design/20261107-i013-unlock-all.md).
void reexceed_set_unlock_all(int enabled);
int reexceed_host_unlock_all(void);

// Built-in autoplay toggle (issue #13, docs/design/20261107-i013-unlock-all.md).
void reexceed_set_autoplay(int enabled);
int reexceed_host_autoplay(void);

// Implemented in decomp/unlock_all.c when recompiled code is present: applies
// or restores temporary unlocks and autoplay on live guest structures.
void reexceed_decomp_sync_unlock_all(void);
void reexceed_decomp_sync_autoplay(void);

// Full-size picture (issue #14, docs/design/20261107-i014-full-size-picture.md):
// 1 (the default) draws 2D and 3D at the back buffer size instead of the
// title's TV-overscan virtual size (1.2x, 1.1x on the HDTV AV pack).
void reexceed_set_full_size_picture(int enabled);
int reexceed_host_full_size_picture(void);
// Implemented in decomp/full_size_picture.c when recompiled code is present.
void reexceed_decomp_sync_full_size_picture(void);

#ifdef __cplusplus
}
#endif

#endif  // REEXCEED_EXCEED_HOST_H
