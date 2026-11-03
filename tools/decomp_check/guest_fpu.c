/* reexceed_decomp_check: the guest state the rexbox ABI does not expose, read
 * and set through the generated code's own header (recomp_types.h): the x87
 * control word, stack top and slots, and the SEH frame bridge. */

#include "recomp_types.h"

#include "guest_fpu.h"

void reexceed_check_set_fpu(uint16_t control_word, uint32_t top)
{
    g_fp_control_word = control_word;
    g_fp_top = (int)(top & 7u);
}

void reexceed_check_get_fpu(uint16_t* control_word, uint32_t* top)
{
    *control_word = g_fp_control_word;
    *top = (uint32_t)g_fp_top;
}

void reexceed_check_set_fpu_stack(const double slots[8])
{
    for (int i = 0; i < 8; ++i)
    {
        g_fp_stack[i] = slots[i];
    }
}

void reexceed_check_get_fpu_stack(double slots[8])
{
    for (int i = 0; i < 8; ++i)
    {
        slots[i] = g_fp_stack[i];
    }
}

void reexceed_check_set_seh_ebp(uint32_t value)
{
    g_seh_ebp = value;
}

uint32_t reexceed_check_get_seh_ebp(void)
{
    return g_seh_ebp;
}
