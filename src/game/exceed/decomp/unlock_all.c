/* Decompiled unlock-boundary functions and host adapters for temporary
 * unlock-all and built-in autoplay (issue #13,
 * docs/design/20261107-i013-unlock-all.md,
 * docs/analysis/exceed-xbe-overview.md section 14).
 *
 * When reexceed_host_unlock_all() and reexceed_host_autoplay() are 0,
 * sub_000278A0, sub_00033340, and sub_00038D60 delegate directly to their
 * lifted bodies (sub_XXXXXXXX_gen). When enabled, all songs, charts, speed
 * and note modifiers, and sub-modes are unlocked in memory while in Mode
 * Select or Song Select and restored upon leaving Song Select (sub_00033340),
 * so post-stage unlock evaluation (sub_00027960) and disk saves (sub_00072F10)
 * always operate on the clean original save buffer. */

#include "recomp_types.h"
#include "reexceed/exceed_host.h"

extern void sub_000278A0_gen(void);
extern void sub_00033340_gen(void);
extern void sub_00038D60_gen(void);

extern void sub_000369F0(void);
extern void sub_00038CE0(void);

enum
{
    kSavePtrAddr = 0x0084865Cu,
    kSaveShadowOffset = 0x2800u,
    kPlayPtrAddr = 0x00825114u,
    kPlayAutoplayOffset = 0x39Du,
    kSongTableAddr = 0x001C5D48u,
    kSongCurrentLockAddr = 0x001C5D88u,
    kSongDefaultLockAddr = 0x001C5D8Cu,
    kSongDisabledFlagAddr = 0x001C5D90u,
    kSongEntryStride = 0x50u,
    kSongTableBytes = 0x5000u,
    kModeMenuVtable = 0x001B91DCu,
    kSubModeCount = 11u,
    kOptionUnlockCount = 24u
};

/* The 24 speed and note modifier flags in the save buffer that start locked
 * (0) in sub_000726F0: 7 category flags (+0x1F75..+0x1F7D) and 17 sub-item
 * flags (+0x1F93..+0x1FBD). */
static const uint16_t kOptionOffsets[kOptionUnlockCount] = {
    0x1F75u, 0x1F76u, 0x1F79u, 0x1F7Au, 0x1F7Bu, 0x1F7Cu, 0x1F7Du, 0x1F93u,
    0x1F94u, 0x1F98u, 0x1F99u, 0x1FA7u, 0x1FA8u, 0x1FACu, 0x1FADu, 0x1FB1u,
    0x1FB2u, 0x1FB3u, 0x1FB6u, 0x1FB7u, 0x1FB8u, 0x1FBBu, 0x1FBCu, 0x1FBDu
};

static int s_guest_ready = 0;
static int s_in_song_select = 0;
static int s_options_overridden = 0;
static int s_autoplay_overridden = 0;
static uint8_t s_saved_options[kOptionUnlockCount];
static uint8_t s_saved_shadow_options[kOptionUnlockCount];
static uint32_t s_last_mode_menu = 0;
static uint32_t s_mode_natural_mask = 0;

static void apply_song_select_unlocks(void)
{
    uint32_t offset = 0;
    for (offset = 0; offset < kSongTableBytes; offset += kSongEntryStride)
    {
        if (MEM32(kSongTableAddr + offset) == 0xFFFFFFFFu)
        {
            break;
        }
        if (MEM8(kSongDisabledFlagAddr + offset) == 0)
        {
            uint32_t mode = 0;
            for (mode = 0; mode < 4u; ++mode)
            {
                MEM8(kSongCurrentLockAddr + offset + mode) = 0;
                MEM8(kSongDefaultLockAddr + offset + mode) = 0;
            }
        }
    }

    const uint32_t save_ptr = MEM32(kSavePtrAddr);
    if (save_ptr != 0)
    {
        uint32_t i = 0;
        if (!s_options_overridden)
        {
            for (i = 0; i < kOptionUnlockCount; ++i)
            {
                const uint32_t opt = kOptionOffsets[i];
                s_saved_options[i] = MEM8(save_ptr + opt);
                s_saved_shadow_options[i] = MEM8(save_ptr + kSaveShadowOffset + opt);
            }
            s_options_overridden = 1;
        }
        for (i = 0; i < kOptionUnlockCount; ++i)
        {
            const uint32_t opt = kOptionOffsets[i];
            MEM8(save_ptr + opt) = 1;
            MEM8(save_ptr + kSaveShadowOffset + opt) = 1;
        }
    }
}

static void restore_song_select_unlocks(void)
{
    if (!s_options_overridden)
    {
        return;
    }

    const uint32_t save_ptr = MEM32(kSavePtrAddr);
    if (save_ptr != 0)
    {
        uint32_t i = 0;
        for (i = 0; i < kOptionUnlockCount; ++i)
        {
            const uint32_t opt = kOptionOffsets[i];
            MEM8(save_ptr + opt) = s_saved_options[i];
            MEM8(save_ptr + kSaveShadowOffset + opt) = s_saved_shadow_options[i];
        }

        uint32_t offset = 0;
        for (offset = 0; offset < kSongTableBytes; offset += kSongEntryStride)
        {
            const uint32_t song_id = MEM32(kSongTableAddr + offset);
            if (song_id == 0xFFFFFFFFu)
            {
                break;
            }
            if (MEM8(kSongDisabledFlagAddr + offset) == 0 && song_id < 0x100u)
            {
                uint32_t mode = 0;
                for (mode = 0; mode < 4u; ++mode)
                {
                    const uint32_t idx = (mode << 8) + song_id;
                    MEM8(kSongCurrentLockAddr + offset + mode) = MEM8(save_ptr + 0x1373u + idx);
                    MEM8(kSongDefaultLockAddr + offset + mode) = MEM8(save_ptr + 0x1773u + idx);
                }
            }
        }
    }
    s_options_overridden = 0;
}

static void sync_mode_menu_unlocks(void)
{
    if (s_last_mode_menu == 0 || MEM32(s_last_mode_menu) != kModeMenuVtable)
    {
        return;
    }

    const int unlock_all = reexceed_host_unlock_all();
    uint32_t count = 0;
    uint32_t mode = 0;
    for (mode = 0; mode < kSubModeCount; ++mode)
    {
        if (unlock_all != 0 || ((s_mode_natural_mask >> mode) & 1u) != 0)
        {
            MEM32(s_last_mode_menu + count * 4u + 0x5Cu) = mode;
            ++count;
        }
    }
    MEM32(s_last_mode_menu + 0x58u) = count;
}

static void sync_autoplay_byte(void)
{
    const int enabled = reexceed_host_autoplay();
    if (!enabled && !s_autoplay_overridden)
    {
        return;
    }

    const uint32_t play_ptr = MEM32(kPlayPtrAddr);
    if (play_ptr != 0)
    {
        MEM8(play_ptr + kPlayAutoplayOffset) = (uint8_t)(enabled != 0 ? 1u : 0u);
    }
    s_autoplay_overridden = enabled != 0 ? 1 : 0;
}

void reexceed_decomp_sync_unlock_all(void)
{
    if (!s_guest_ready)
    {
        return;
    }
    sync_mode_menu_unlocks();
    if (s_in_song_select)
    {
        if (reexceed_host_unlock_all() != 0)
        {
            apply_song_select_unlocks();
        }
        else
        {
            restore_song_select_unlocks();
        }
    }
    else if (reexceed_host_unlock_all() == 0)
    {
        restore_song_select_unlocks();
    }
}

void reexceed_decomp_sync_autoplay(void)
{
    if (!s_guest_ready)
    {
        return;
    }
    sync_autoplay_byte();
}

/* 0x000278A0, cdecl with implicit edi: void (int mode_edi, uint32_t* out_new_song_id)
 *
 * Syncs the current mode's song lock flags from the save buffer (+0x1773 and
 * +0x1373) into the .data song table (0x001C5D8C and 0x001C5D88) and writes
 * the last newly unlocked song ID (or 0xFFFFFFFF) to *out_new_song_id. */
void sub_000278A0(void)
{
    sub_000278A0_gen();

    s_guest_ready = 1;
    s_in_song_select = 1;
    if (reexceed_host_unlock_all() != 0)
    {
        apply_song_select_unlocks();
    }
    sync_autoplay_byte();
}

/* 0x00033340, thiscall: void (SongSelect* ecx)
 *
 * Song Select exit/destructor (vtable[3] at 0x001B7434). Restores any temporary
 * song/modifier unlocks before tearing down Song Select and transitioning to the
 * stage or Mode Select. */
void sub_00033340(void)
{
    s_in_song_select = 0;
    const uint32_t saved_this = g_ecx;
    restore_song_select_unlocks();
    g_ecx = saved_this;
    sub_00033340_gen();
}

/* 0x00038D60, stdcall: void (ModeMenu* menu)
 *
 * Evaluates all 11 sub-modes (0..10) via sub_00038CE0, records newly earned
 * unlocks in save_ptr + 0x1FC0, and populates menu->count (+0x58) and
 * menu->modes[] (+0x5C). */
void sub_00038D60(void)
{
    uint32_t ebp = g_ebp;
    PUSH32(g_esp, ebp);
    ebp = MEM32(g_esp + 8u);
    PUSH32(g_esp, g_esi);
    PUSH32(g_esp, g_edi);
    g_edi = 0;
    MEM32(ebp + 0x58u) = g_edi;

    uint32_t natural_mask = 0;
    do
    {
        g_eax = g_edi;
        PUSH32(g_esp, 0x00038D77u);
        RECOMP_ABI_CALL(0x00038CE0u, sub_00038CE0);
        if (LO8(g_eax) != 0)
        {
            natural_mask |= (1u << g_edi);
            g_eax = MEM32(kSavePtrAddr);
            SET_LO8(g_ecx, MEM8(g_eax + g_edi + 0x1FC0u));
            if (LO8(g_ecx) == 0)
            {
                PUSH32(g_esp, 0);
                MEM8(g_eax + g_edi + 0x1FC0u) = 1;
                g_esi = MEM32(0x00848670u);
                PUSH32(g_esp, g_edi);
                g_ecx = 3;
                PUSH32(g_esp, 0x00038DA6u);
                RECOMP_ABI_CALL(0x000369F0u, sub_000369F0);
            }
            g_eax = MEM32(ebp + 0x58u);
            MEM32(ebp + g_eax * 4u + 0x5Cu) = g_edi;
            MEM32(ebp + 0x58u) = MEM32(ebp + 0x58u) + 1u;
        }
        else if (reexceed_host_unlock_all() != 0)
        {
            const uint32_t count = MEM32(ebp + 0x58u);
            MEM32(ebp + count * 4u + 0x5Cu) = g_edi;
            MEM32(ebp + 0x58u) = count + 1u;
        }
        g_edi++;
    } while ((int32_t)g_edi < (int32_t)kSubModeCount);

    s_guest_ready = 1;
    s_last_mode_menu = ebp;
    s_mode_natural_mask = natural_mask;
    sync_autoplay_byte();

    POP32(g_esp, g_edi);
    POP32(g_esp, g_esi);
    POP32(g_esp, ebp);
    g_esp += 8u; /* ret 4 */
}
