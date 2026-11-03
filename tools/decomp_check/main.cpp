// reexceed_decomp_check: runs each decompiled function and the lifted body it
// replaced (sub_X_gen) from identical guest states and compares what they
// leave behind: the general registers, g_seh_ebp, XMM0..7, the x87 control
// word, stack top and all eight stack slots (bit for bit), and all of guest
// memory.
//
// Usage: reexceed_decomp_check
// Exit code 0 when every case matches.

#include "guest_fpu.h"

#include "rexbox/recomp/recomp_abi.h"
#include "rexbox/runtime/guest_memory.h"

#include <cstdio>
#include <cstring>
#include <functional>
#include <string>
#include <vector>

// Decompiled functions and their lifted bodies, declared as
//     extern "C" void sub_XXXXXXXX(void);
//     extern "C" void sub_XXXXXXXX_gen(void);
// next to the suite that checks them. None yet (src/game/exceed/decomp/).

namespace
{

using rexbox::runtime::GuestAddress;
using rexbox::runtime::GuestMemory;
using GuestFunction = void (*)();

constexpr GuestAddress kStackTop = 0x00F00000;
constexpr GuestAddress kEsp = kStackTop - 0x100;  // the return address; room below for frames
// x87 control words: all exceptions masked, rounding to nearest, PC=24/53/64.
constexpr uint16_t kPc24 = 0x007F;
[[maybe_unused]] constexpr uint16_t kPc53 = 0x027F;
[[maybe_unused]] constexpr uint16_t kPc64 = 0x037F;

[[maybe_unused]] void WriteU32(GuestMemory& memory, GuestAddress address, uint32_t value)
{
    std::memcpy(memory.Translate(address), &value, 4);
}

[[maybe_unused]] void WriteF32(GuestMemory& memory, GuestAddress address, float value)
{
    std::memcpy(memory.Translate(address), &value, 4);
}

struct GuestState
{
    RexboxRecompGprs gprs = {};
    uint32_t xmm[8][4] = {};
    uint16_t control_word = kPc24;
    uint32_t fpu_top = 0;
    double fpu_slots[8] = {};
    uint32_t seh_ebp = 0;
};

struct Case
{
    std::string name;
    std::function<void(GuestMemory&, GuestState&)> setup;
};

struct Outcome
{
    GuestState state;
    std::vector<uint8_t> memory;
};

Outcome RunFrom(GuestMemory& memory, const std::vector<uint8_t>& initial_memory, const GuestState& initial,
                GuestFunction function)
{
    std::memcpy(memory.base(), initial_memory.data(), initial_memory.size());
    rexbox_recomp_reset_thread(initial.gprs.esp, 0);
    rexbox_recomp_set_gprs(&initial.gprs);
    for (int i = 0; i < 8; ++i)
    {
        rexbox_recomp_set_xmm(i, initial.xmm[i]);
    }
    reexceed_check_set_fpu(initial.control_word, initial.fpu_top);
    reexceed_check_set_fpu_stack(initial.fpu_slots);
    reexceed_check_set_seh_ebp(initial.seh_ebp);
    function();
    Outcome outcome;
    rexbox_recomp_get_gprs(&outcome.state.gprs);
    for (int i = 0; i < 8; ++i)
    {
        rexbox_recomp_get_xmm(i, outcome.state.xmm[i]);
    }
    reexceed_check_get_fpu(&outcome.state.control_word, &outcome.state.fpu_top);
    reexceed_check_get_fpu_stack(outcome.state.fpu_slots);
    outcome.state.seh_ebp = reexceed_check_get_seh_ebp();
    outcome.memory.assign(memory.base(), memory.base() + memory.size());
    return outcome;
}

bool Compare(const char* function, const Case& test, const Outcome& decompiled, const Outcome& lifted)
{
    bool same = true;
    const uint32_t* a = &decompiled.state.gprs.eax;
    const uint32_t* b = &lifted.state.gprs.eax;
    static const char* kNames[] = {"eax", "ecx", "edx", "ebx", "esp", "ebp", "esi", "edi"};
    for (int i = 0; i < 8; ++i)
    {
        if (a[i] != b[i])
        {
            std::printf("  %s %s: %s decompiled %08X lifted %08X\n", function, test.name.c_str(), kNames[i], a[i], b[i]);
            same = false;
        }
    }
    for (int i = 0; i < 8; ++i)
    {
        if (std::memcmp(decompiled.state.xmm[i], lifted.state.xmm[i], 16) != 0)
        {
            const uint32_t* x = decompiled.state.xmm[i];
            const uint32_t* y = lifted.state.xmm[i];
            std::printf("  %s %s: xmm%d decompiled %08X %08X %08X %08X lifted %08X %08X %08X %08X\n", function,
                        test.name.c_str(), i, x[0], x[1], x[2], x[3], y[0], y[1], y[2], y[3]);
            same = false;
        }
    }
    if (decompiled.state.control_word != lifted.state.control_word ||
        decompiled.state.fpu_top != lifted.state.fpu_top)
    {
        std::printf("  %s %s: x87 control/top decompiled %04X/%u lifted %04X/%u\n", function, test.name.c_str(),
                    decompiled.state.control_word, decompiled.state.fpu_top, lifted.state.control_word,
                    lifted.state.fpu_top);
        same = false;
    }
    for (int i = 0; i < 8; ++i)
    {
        if (std::memcmp(&decompiled.state.fpu_slots[i], &lifted.state.fpu_slots[i], 8) != 0)
        {
            std::printf("  %s %s: x87 slot %d decompiled %.17g lifted %.17g\n", function, test.name.c_str(), i,
                        decompiled.state.fpu_slots[i], lifted.state.fpu_slots[i]);
            same = false;
        }
    }
    if (decompiled.state.seh_ebp != lifted.state.seh_ebp)
    {
        std::printf("  %s %s: g_seh_ebp decompiled %08X lifted %08X\n", function, test.name.c_str(),
                    decompiled.state.seh_ebp, lifted.state.seh_ebp);
        same = false;
    }
    if (decompiled.memory != lifted.memory)
    {
        int shown = 0;
        for (size_t i = 0; i < decompiled.memory.size() && shown < 8; i += 4)
        {
            uint32_t x = 0;
            uint32_t y = 0;
            std::memcpy(&x, &decompiled.memory[i], 4);
            std::memcpy(&y, &lifted.memory[i], 4);
            if (x != y)
            {
                std::printf("  %s %s: memory %08zX decompiled %08X lifted %08X\n", function, test.name.c_str(), i, x,
                            y);
                ++shown;
            }
        }
        same = false;
    }
    return same;
}

struct Suite
{
    const char* name;
    GuestFunction decompiled;
    GuestFunction lifted;
    std::vector<Case> cases;
};

// Registers and XMM lanes with recognizable values, and stdcall arguments.
[[maybe_unused]] void Frame(GuestMemory& m, GuestState& s, uint32_t a1, uint32_t a2, uint32_t a3)
{
    RexboxRecompGprs& r = s.gprs;
    r.eax = 0xDEADBEEF;
    r.ecx = 0x0C0C0C0C;
    r.edx = 0x11111111;
    r.ebx = 0x22222222;
    r.esi = 0x33333333;
    r.edi = 0x44444444;
    r.ebp = 0x55555555;
    r.esp = kEsp;
    s.seh_ebp = 0x66666666;
    // Stale x87 slots, so a slot a function leaves different shows.
    for (int i = 0; i < 8; ++i)
    {
        s.fpu_slots[i] = 1000.0 + i;
    }
    for (uint32_t i = 0; i < 8; ++i)
    {
        for (uint32_t j = 0; j < 4; ++j)
        {
            s.xmm[i][j] = 0x7F000000u | (i << 8) | j;
        }
    }
    // Stack contents the callee may overwrite, so a store it misses shows.
    for (GuestAddress a = kEsp - 0x80; a < kEsp + 0x20; a += 4)
    {
        WriteU32(m, a, 0xA5A50000u | (a & 0xFFFF));
    }
    WriteU32(m, kEsp, 0x00012345);  // return address
    WriteU32(m, kEsp + 4, a1);
    WriteU32(m, kEsp + 8, a2);
    WriteU32(m, kEsp + 12, a3);
}

// Suites, one per decompiled function: Suite{"sub_XXXXXXXX", &sub_XXXXXXXX,
// &sub_XXXXXXXX_gen, cases}, each case a name and a setup that fills guest
// memory and the starting state (Frame() gives recognizable registers and
// stdcall arguments).
std::vector<Suite> Suites()
{
    return {};
}

}  // namespace

int main()
{
    // 16 MiB is enough for these cases and keeps the per-case copies cheap.
    GuestMemory memory;
    if (!memory.Initialize(16u * 1024u * 1024u))
    {
        std::fprintf(stderr, "cannot reserve guest memory\n");
        return 2;
    }
    rexbox_recomp_set_memory(reinterpret_cast<ptrdiff_t>(memory.base()), 0, 0);

    const std::vector<Suite> suites = Suites();

    size_t total = 0;
    int failures = 0;
    for (const Suite& suite : suites)
    {
        for (const Case& test : suite.cases)
        {
            std::memset(memory.base(), 0, memory.size());
            GuestState initial;
            test.setup(memory, initial);
            const std::vector<uint8_t> initial_memory(memory.base(), memory.base() + memory.size());
            const Outcome decompiled = RunFrom(memory, initial_memory, initial, suite.decompiled);
            const Outcome lifted = RunFrom(memory, initial_memory, initial, suite.lifted);
            const bool same = Compare(suite.name, test, decompiled, lifted);
            std::printf("%s %s %s\n", same ? "match" : "DIFF ", suite.name, test.name.c_str());
            failures += same ? 0 : 1;
            ++total;
        }
    }
    std::printf("%zu case(s), %d difference(s)\n", total, failures);
    return failures == 0 ? 0 : 1;
}
