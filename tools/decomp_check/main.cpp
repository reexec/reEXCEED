// reexceed_decomp_check: runs each decompiled function and the lifted body it
// replaced (sub_X_gen) from identical guest states and compares what they
// leave behind: the general registers, g_seh_ebp, XMM0..7, the x87 control
// word, stack top and all eight stack slots (bit for bit), and all of guest
// memory.
//
// Usage: reexceed_decomp_check
// Exit code 0 when every case matches.

#include "guest_fpu.h"

#include "reexceed/exceed_host.h"
#include "rexbox/platform/guarded_call.h"
#include "rexbox/recomp/recomp_abi.h"
#include "rexbox/runtime/guest_memory.h"

#include <cstdio>
#include <cstring>
#include <functional>
#include <string>
#include <vector>

// Decompiled functions and their lifted bodies (src/game/exceed/decomp/).
extern "C" void sub_000278A0(void);
extern "C" void sub_000278A0_gen(void);
extern "C" void sub_00033340(void);
extern "C" void sub_00033340_gen(void);
extern "C" void sub_00038D60(void);
extern "C" void sub_00038D60_gen(void);
extern "C" void sub_00075F10(void);
extern "C" void sub_00075F10_gen(void);

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
    auto fn_wrapper = [](void* ctx) {
        auto fn = reinterpret_cast<GuestFunction>(ctx);
        fn();
    };
    const auto res = rexbox::platform::RunGuarded(fn_wrapper, reinterpret_cast<void*>(function));
    if (res.outcome != rexbox::platform::GuardedOutcome::kReturned)
    {
        std::printf("RunGuarded outcome=%d stop_code=%08X fault=%08X at %p\n",
                    static_cast<int>(res.outcome), res.stop_code, res.fault_code,
                    reinterpret_cast<void*>(res.exception_address));
        std::fflush(stdout);
    }
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

void PopulateJumpTable(GuestMemory& m)
{
    static const uint32_t kTable[11] = {
        0x00038CECu, 0x00038CECu, 0x00038CECu, 0x00038CEFu, 0x00038D16u,
        0x00038D16u, 0x00038D03u, 0x00038CECu, 0x00038CECu, 0x00038CECu, 0x00038CECu
    };
    for (int i = 0; i < 11; ++i)
    {
        WriteU32(m, 0x00038D28 + i * 4, kTable[i]);
    }
}

// The globals sub_000765E0 leaves for sub_00075F10 (issue #14): a 640x480
// back buffer, the AV pack, the 1.1/1.2 scale constants, the virtual size the
// original computes (768x576 for pack 0), and a device for SetTransform.
void SetUpVirtualSize(GuestMemory& m, uint32_t av_pack)
{
    WriteU32(m, 0x00842334, 640);
    WriteU32(m, 0x00842398, 480);
    WriteU32(m, 0x00842338, av_pack);
    WriteF32(m, 0x001BD3A8, 1.0f);
    WriteF32(m, 0x001BD3BC, 1.1f);
    WriteF32(m, 0x001BD46C, 1.2f);
    const float scale = av_pack == 1 ? 1.1f : 1.2f;
    WriteF32(m, 0x00842358, 640.0f * scale);
    WriteF32(m, 0x00842354, 480.0f * scale);
    WriteU32(m, 0x00185A88, 0x00400000);  // g_pDevice
}

float ReadF32(GuestMemory& m, GuestAddress address)
{
    float value = 0.0f;
    std::memcpy(&value, m.Translate(address), 4);
    return value;
}

// Runs a guest function guarded: sub_00075F10 ends in the D3D SetTransform
// HLE, which stops here (no HLE device in this tool) after the adapter's work.
void CallGuest(GuestFunction function)
{
    RexboxRecompGprs gprs = {};
    gprs.esp = kEsp;
    rexbox_recomp_reset_thread(gprs.esp, 0);
    rexbox_recomp_set_gprs(&gprs);
    rexbox::platform::RunGuarded([](void* ctx) { reinterpret_cast<GuestFunction>(ctx)(); },
                                 reinterpret_cast<void*>(function));
}

// Suites, one per decompiled function: Suite{"sub_XXXXXXXX", &sub_XXXXXXXX,
// &sub_XXXXXXXX_gen, cases}, each case a name and a setup that fills guest
// memory and the starting state (Frame() gives recognizable registers and
// stdcall arguments).
std::vector<Suite> Suites()
{
    std::vector<Suite> suites;

    // sub_000278A0: Song Select enter lock sync
    suites.push_back(Suite{
        "sub_000278A0",
        &sub_000278A0,
        &sub_000278A0_gen,
        {
            Case{"empty_song_table", [](GuestMemory& m, GuestState& s) {
                reexceed_set_unlock_all(0);
                reexceed_set_autoplay(0);
                constexpr GuestAddress kSavePtr = 0x00300000;
                constexpr GuestAddress kOutNewSong = 0x00200000;
                Frame(m, s, kOutNewSong, 0, 0);
                s.gprs.edi = 0;  // mode 0 (Arcade)
                WriteU32(m, 0x0084865C, kSavePtr);
                WriteU32(m, 0x001C5D48, 0xFFFFFFFF);  // end of song table
                WriteU32(m, kOutNewSong, 0x12345678);
            }},
        }
    });

    // sub_00038D60: Mode menu evaluation
    suites.push_back(Suite{
        "sub_00038D60",
        &sub_00038D60,
        &sub_00038D60_gen,
        {
            Case{"default_locked", [](GuestMemory& m, GuestState& s) {
                reexceed_set_unlock_all(0);
                reexceed_set_autoplay(0);
                constexpr GuestAddress kMenu = 0x00500000;
                constexpr GuestAddress kSavePtr = 0x00600000;
                Frame(m, s, kMenu, 0, 0);
                PopulateJumpTable(m);
                WriteU32(m, 0x0084865C, kSavePtr);
                WriteU32(m, 0x001C5D48, 0xFFFFFFFF);  // end of song table
                WriteU32(m, kMenu, 0x001B91DC);      // vtable
                WriteU32(m, kMenu + 0x58, 0);        // count
                for (int i = 0; i < 11; ++i)
                {
                    *reinterpret_cast<uint8_t*>(m.Translate(kSavePtr + 0x1FC0 + i)) = 1; // popups already shown
                }
            }},
        }
    });

    // sub_00075F10: 2D setup behind the full-size picture adapter; with the
    // setting off and nothing overridden it is the lifted body.
    suites.push_back(Suite{
        "sub_00075F10",
        &sub_00075F10,
        &sub_00075F10_gen,
        {
            Case{"full_size_off_pack0", [](GuestMemory& m, GuestState& s) {
                reexceed_set_full_size_picture(0);
                Frame(m, s, 0, 0, 0);
                SetUpVirtualSize(m, 0);
            }},
        }
    });

    return suites;
}

bool TestUnlockAll(GuestMemory& memory)
{
    bool ok = true;
    std::memset(memory.base(), 0, memory.size());
    constexpr GuestAddress kSavePtr = 0x00600000;
    constexpr GuestAddress kSaveShadow = kSavePtr + 0x2800;
    constexpr GuestAddress kMenu = 0x00500000;
    constexpr GuestAddress kSongTable = 0x001C5D48;
    constexpr GuestAddress kOutNewSong = 0x00200000;
    constexpr uint16_t kOptionOffsets[24] = {
        0x1F75u, 0x1F76u, 0x1F79u, 0x1F7Au, 0x1F7Bu, 0x1F7Cu, 0x1F7Du, 0x1F93u,
        0x1F94u, 0x1F98u, 0x1F99u, 0x1FA7u, 0x1FA8u, 0x1FACu, 0x1FADu, 0x1FB1u,
        0x1FB2u, 0x1FB3u, 0x1FB6u, 0x1FB7u, 0x1FB8u, 0x1FBBu, 0x1FBCu, 0x1FBDu
    };

    PopulateJumpTable(memory);
    WriteU32(memory, 0x0084865C, kSavePtr);
    WriteU32(memory, kMenu, 0x001B91DC);
    WriteU32(memory, kMenu + 0x58, 0);
    WriteF32(memory, 0x001BD3A8, 1.0f);
    WriteF32(memory, 0x001BD50C, 2.0f);
    WriteF32(memory, 0x00842354, 640.0f);
    WriteF32(memory, 0x00842358, 480.0f);

    // Song 0: active, initially locked
    WriteU32(memory, kSongTable, 0);
    WriteU32(memory, kSongTable + 0x48, 0); // enabled
    for (int mode = 0; mode < 4; ++mode)
    {
        *reinterpret_cast<uint8_t*>(memory.Translate(kSongTable + 0x40 + mode)) = 1;
        *reinterpret_cast<uint8_t*>(memory.Translate(kSongTable + 0x44 + mode)) = 1;
        *reinterpret_cast<uint8_t*>(memory.Translate(kSavePtr + 0x1373 + (mode << 8))) = 1;
        *reinterpret_cast<uint8_t*>(memory.Translate(kSavePtr + 0x1773 + (mode << 8))) = 1;
        *reinterpret_cast<uint8_t*>(memory.Translate(kSaveShadow + 0x1373 + (mode << 8))) = 1;
        *reinterpret_cast<uint8_t*>(memory.Translate(kSaveShadow + 0x1773 + (mode << 8))) = 1;
    }
    // Song 1: end of table
    WriteU32(memory, kSongTable + 0x50, 0xFFFFFFFF);

    for (int i = 0; i < 11; ++i)
    {
        const uint8_t natural = (i < 3 || i >= 7) ? 1 : 0;
        *reinterpret_cast<uint8_t*>(memory.Translate(kSavePtr + 0x1FC0 + i)) = natural;
        *reinterpret_cast<uint8_t*>(memory.Translate(kSaveShadow + 0x1FC0 + i)) = natural;
    }

    for (int i = 0; i < 24; ++i)
    {
        *reinterpret_cast<uint8_t*>(memory.Translate(kSavePtr + kOptionOffsets[i])) = 0;
        *reinterpret_cast<uint8_t*>(memory.Translate(kSaveShadow + kOptionOffsets[i])) = 0;
    }

    // Step 1: Turn ON unlock-all
    std::printf("  TestUnlockAll: Step 1 (Turn ON)...\n");
    std::fflush(stdout);
    reexceed_set_unlock_all(1);
    if (reexceed_host_unlock_all() != 1)
    {
        std::printf("FAIL: reexceed_host_unlock_all != 1\n");
        ok = false;
    }

    // Mode menu population
    std::printf("  TestUnlockAll: Mode menu...\n");
    std::fflush(stdout);
    {
        RexboxRecompGprs gprs = {};
        gprs.esp = kEsp;
        WriteU32(memory, kEsp, 0x00012345);
        WriteU32(memory, kEsp + 4, kMenu);
        rexbox_recomp_reset_thread(gprs.esp, 0);
        rexbox_recomp_set_gprs(&gprs);
        sub_00038D60();

        uint32_t count = 0;
        std::memcpy(&count, memory.Translate(kMenu + 0x58), 4);
        if (count != 11)
        {
            std::printf("FAIL: unlock_all mode count expected 11, got %u\n", count);
            ok = false;
        }
        const uint8_t mode3_save = *reinterpret_cast<uint8_t*>(memory.Translate(kSavePtr + 0x1FC0 + 3));
        if (mode3_save != 0)
        {
            std::printf("FAIL: save_ptr sub-mode 3 was modified by unlock-all\n");
            ok = false;
        }
    }

    // Song select enter
    std::printf("  TestUnlockAll: Song select enter...\n");
    std::fflush(stdout);
    {
        RexboxRecompGprs gprs = {};
        gprs.esp = kEsp;
        gprs.edi = 0;
        WriteU32(memory, kEsp, 0x00012345);
        WriteU32(memory, kEsp + 4, kOutNewSong);
        rexbox_recomp_reset_thread(gprs.esp, 0);
        rexbox_recomp_set_gprs(&gprs);
        sub_000278A0();

        for (int mode = 0; mode < 4; ++mode)
        {
            const uint8_t cur_lock = *reinterpret_cast<uint8_t*>(memory.Translate(kSongTable + 0x40 + mode));
            const uint8_t def_lock = *reinterpret_cast<uint8_t*>(memory.Translate(kSongTable + 0x44 + mode));
            if (cur_lock != 0 || def_lock != 0)
            {
                std::printf("FAIL: song 0 mode %d lock not 0: cur=%u def=%u\n", mode, cur_lock, def_lock);
                ok = false;
            }
        }
        for (int i = 0; i < 24; ++i)
        {
            const uint8_t opt = *reinterpret_cast<uint8_t*>(memory.Translate(kSavePtr + kOptionOffsets[i]));
            const uint8_t shadow_opt = *reinterpret_cast<uint8_t*>(memory.Translate(kSaveShadow + kOptionOffsets[i]));
            if (opt != 1 || shadow_opt != 1)
            {
                std::printf("FAIL: option %d not unlocked in save/shadow: opt=%u shadow=%u\n", i, opt, shadow_opt);
                ok = false;
            }
        }
    }

    // Step 2: Live toggle OFF while in Song Select
    std::printf("  TestUnlockAll: Live toggle OFF...\n");
    std::fflush(stdout);
    reexceed_set_unlock_all(0);
    if (reexceed_host_unlock_all() != 0)
    {
        std::printf("FAIL: reexceed_host_unlock_all != 0\n");
        ok = false;
    }
    for (int i = 0; i < 24; ++i)
    {
        const uint8_t opt = *reinterpret_cast<uint8_t*>(memory.Translate(kSavePtr + kOptionOffsets[i]));
        const uint8_t shadow_opt = *reinterpret_cast<uint8_t*>(memory.Translate(kSaveShadow + kOptionOffsets[i]));
        if (opt != 0 || shadow_opt != 0)
        {
            std::printf("FAIL: live toggle OFF did not restore option %d: opt=%u shadow=%u\n", i, opt, shadow_opt);
            ok = false;
        }
    }
    for (int mode = 0; mode < 4; ++mode)
    {
        const uint8_t cur_lock = *reinterpret_cast<uint8_t*>(memory.Translate(kSongTable + 0x40 + mode));
        if (cur_lock != 1)
        {
            std::printf("FAIL: live toggle OFF did not restore song 0 mode %d lock: %u\n", mode, cur_lock);
            ok = false;
        }
    }
    uint32_t count = 0;
    std::memcpy(&count, memory.Translate(kMenu + 0x58), 4);
    if (count != 7)
    {
        std::printf("FAIL: live toggle OFF mode count expected 7, got %u\n", count);
        ok = false;
    }

    // Step 3: Turn ON again, verify unlocked, then toggle OFF and verify clean restore
    std::printf("  TestUnlockAll: Turn ON again...\n");
    std::fflush(stdout);
    reexceed_set_unlock_all(1);
    for (int i = 0; i < 24; ++i)
    {
        const uint8_t opt = *reinterpret_cast<uint8_t*>(memory.Translate(kSavePtr + kOptionOffsets[i]));
        const uint8_t shadow_opt = *reinterpret_cast<uint8_t*>(memory.Translate(kSaveShadow + kOptionOffsets[i]));
        if (opt != 1 || shadow_opt != 1)
        {
            std::printf("FAIL: Turn ON again did not set option %d: opt=%u shadow=%u\n", i, opt, shadow_opt);
            ok = false;
        }
    }
    for (int mode = 0; mode < 4; ++mode)
    {
        const uint8_t cur_lock = *reinterpret_cast<uint8_t*>(memory.Translate(kSongTable + 0x40 + mode));
        if (cur_lock != 0)
        {
            std::printf("FAIL: Turn ON again did not set song 0 mode %d lock to 0: %u\n", mode, cur_lock);
            ok = false;
        }
    }

    std::printf("  TestUnlockAll: Final restore and dirty check verification...\n");
    std::fflush(stdout);
    reexceed_set_unlock_all(0);
    for (int i = 0; i < 24; ++i)
    {
        const uint8_t opt = *reinterpret_cast<uint8_t*>(memory.Translate(kSavePtr + kOptionOffsets[i]));
        const uint8_t shadow_opt = *reinterpret_cast<uint8_t*>(memory.Translate(kSaveShadow + kOptionOffsets[i]));
        if (opt != 0 || shadow_opt != 0)
        {
            std::printf("FAIL: final restore did not restore option %d: opt=%u shadow=%u\n", i, opt, shadow_opt);
            ok = false;
        }
    }
    const uint8_t cur_lock = *reinterpret_cast<uint8_t*>(memory.Translate(kSongTable + 0x40));
    if (cur_lock != 1)
    {
        std::printf("FAIL: final restore did not restore song 0 lock: %u\n", cur_lock);
        ok = false;
    }

    // Verify save_ptr exactly matches save_shadow across all 0x2800 bytes (no dirty save!)
    bool shadow_matches = true;
    for (size_t i = 0; i < 0x2800; ++i)
    {
        uint8_t a = *reinterpret_cast<uint8_t*>(memory.Translate(kSavePtr + i));
        uint8_t b = *reinterpret_cast<uint8_t*>(memory.Translate(kSaveShadow + i));
        if (a != b)
        {
            std::printf("Diff at offset +0x%04X: save_ptr=%02X shadow=%02X\n", (unsigned)i, a, b);
            shadow_matches = false;
        }
    }
    if (!shadow_matches)
    {
        std::printf("FAIL: save_ptr does not match shadow copy! Dirty disk write would be triggered!\n");
        ok = false;
    }

    reexceed_set_unlock_all(0);
    return ok;
}

bool TestFullSizePicture(GuestMemory& memory)
{
    bool ok = true;
    const auto expect = [&](const char* step, float width, float height) {
        const float w = ReadF32(memory, 0x00842358);
        const float h = ReadF32(memory, 0x00842354);
        if (w != width || h != height)
        {
            std::printf("FAIL: %s: virtual size %gx%g, expected %gx%g\n", step, w, h, width, height);
            ok = false;
        }
    };
    for (uint32_t pack = 0; pack <= 1; ++pack)
    {
        std::memset(memory.base(), 0, memory.size());
        SetUpVirtualSize(memory, pack);
        const float original_width = ReadF32(memory, 0x00842358);
        const float original_height = ReadF32(memory, 0x00842354);

        reexceed_set_full_size_picture(1);
        CallGuest(&sub_00075F10);
        expect("on, 2D setup", 640.0f, 480.0f);

        reexceed_set_full_size_picture(0);  // live toggle: restores at once
        expect("off, live", original_width, original_height);

        reexceed_set_full_size_picture(1);
        expect("on, live", 640.0f, 480.0f);
        reexceed_set_full_size_picture(0);
        CallGuest(&sub_00075F10);
        expect("off, 2D setup", original_width, original_height);

        // Off and not overridden: guest memory is left alone.
        WriteF32(memory, 0x00842358, 1.0f);
        CallGuest(&sub_00075F10);
        expect("off, untouched", 1.0f, original_height);
    }
    reexceed_set_full_size_picture(1);
    return ok;
}

bool TestAutoplay(GuestMemory& memory)
{
    bool ok = true;
    std::memset(memory.base(), 0, memory.size());
    constexpr GuestAddress kPlayPtr = 0x00700000;
    WriteU32(memory, 0x00825114, kPlayPtr);

    reexceed_set_autoplay(1);
    if (reexceed_host_autoplay() != 1)
    {
        std::printf("FAIL: reexceed_host_autoplay != 1\n");
        ok = false;
    }
    uint8_t flag = *reinterpret_cast<uint8_t*>(memory.Translate(kPlayPtr + 0x39D));
    if (flag != 1)
    {
        std::printf("FAIL: autoplay byte expected 1, got %u\n", flag);
        ok = false;
    }

    reexceed_set_autoplay(0);
    if (reexceed_host_autoplay() != 0)
    {
        std::printf("FAIL: reexceed_host_autoplay != 0\n");
        ok = false;
    }
    flag = *reinterpret_cast<uint8_t*>(memory.Translate(kPlayPtr + 0x39D));
    if (flag != 0)
    {
        std::printf("FAIL: autoplay byte expected 0, got %u\n", flag);
        ok = false;
    }

    return ok;
}

}  // namespace

int main()
{
    std::setvbuf(stdout, nullptr, _IONBF, 0);
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
            std::printf("Running suite %s case %s (decompiled)...\n", suite.name, test.name.c_str());
            std::fflush(stdout);
            const Outcome decompiled = RunFrom(memory, initial_memory, initial, suite.decompiled);
            std::printf("Running suite %s case %s (lifted)...\n", suite.name, test.name.c_str());
            std::fflush(stdout);
            const Outcome lifted = RunFrom(memory, initial_memory, initial, suite.lifted);
            const bool same = Compare(suite.name, test, decompiled, lifted);
            std::printf("%s %s %s\n", same ? "match" : "DIFF ", suite.name, test.name.c_str());
            failures += same ? 0 : 1;
            ++total;
        }
    }
    std::printf("%zu case(s), %d difference(s)\n", total, failures);

    std::printf("Running functional verification for unlock-all and autoplay...\n");
    if (!TestUnlockAll(memory))
    {
        std::printf("TestUnlockAll failed!\n");
        failures++;
    }
    else
    {
        std::printf("TestUnlockAll passed!\n");
    }

    if (!TestAutoplay(memory))
    {
        std::printf("TestAutoplay failed!\n");
        failures++;
    }
    else
    {
        std::printf("TestAutoplay passed!\n");
    }

    if (!TestFullSizePicture(memory))
    {
        std::printf("TestFullSizePicture failed!\n");
        failures++;
    }
    else
    {
        std::printf("TestFullSizePicture passed!\n");
    }

    return failures == 0 ? 0 : 1;
}
