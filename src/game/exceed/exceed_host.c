/* The host hooks of include/reexceed/exceed_host.h: defined here, outside
 * decomp/, because the decompiled sources are compiled only next to the
 * generated code, while the application sets the hooks in every build. */
#include "reexceed/exceed_host.h"

float (*reexceed_host_horizontal_view_scale)(void) = 0;
uint32_t (*reexceed_host_apu_sample_count)(void) = 0;
