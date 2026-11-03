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

#ifdef __cplusplus
}
#endif

#endif  // REEXCEED_EXCEED_HOST_H
