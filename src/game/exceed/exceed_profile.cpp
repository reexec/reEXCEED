#include "reexceed/exceed_profile.h"

#include "rexbox/recomp/recomp_hle_entry.h"
#include "rexbox/runtime/hle_registry.h"
#include "rexbox/xbe/xbe_image.h"

#include <array>
#include <cstdint>

namespace reexceed
{

namespace
{

// Certificate identity of each known build (docs/analysis/exceed-xbe-overview.md).
//
// The North America build is the primary target (MD5
// 575f861eb700e86a82c0ad530dc4a310, XDK 5849), registered with
// scripts/recomp/register_xbe.py, which also keeps the MD5 in
// src/game/exceed/recomp/variants.json. An entry with title id 0 is a
// placeholder IdentifyXbe skips.
constexpr std::array<XbeVariant, 1> kVariants = {{
    // Primary target: the North America retail build's default.xbe (region 1).
    {"North America", 0x41440002, 0x42F8EC5C, 0x00000001, 0x42EDCD9F},
}};

struct XdkInternal
{
    const char* name;
    uint32_t address;
};

// XDK internals XbSymbolDatabase does not name, found by reading the lifted
// library (docs/analysis/exceed-xbe-overview.md section 9). "D3D_g_Device" is
// D3D8LTCG's static device: the lifted Direct3D_CreateDevice
// (0x00177A70) stores 0x001835E0 in g_pDevice ([0x00185A88]), and inlined
// code also writes its push buffer through 0x001835E0 directly, so the HLE
// device must be this object (rexbox design i051). "D3D_InitializeDeviceState"
// is the device's default-state routine the lifted CreateDevice's init
// (0x0017A3D0) calls: identity transforms through SetTransform (the matrix at
// 0x00181E18), then every render state from the default table at 0x001825E4
// through SetRenderState_Simple or the complex setters' table
// (docs/analysis/exceed-xbe-overview.md section 9).
constexpr std::array<XdkInternal, 2> kXdkInternals = {{
    {"D3D_g_Device", 0x001835E0},
    {"D3D_InitializeDeviceState", 0x0017A1E0},
}};

bool Registered(const XbeVariant& variant)
{
    return variant.title_id != 0;
}

}  // namespace

const XbeVariant* IdentifyXbe(const rexbox::xbe::XbeImage& image)
{
    const auto& cert = image.certificate();
    for (const XbeVariant& variant : kVariants)
    {
        if (Registered(variant) && cert.title_id == variant.title_id && cert.time_date == variant.certificate_time &&
            cert.version == variant.certificate_version && image.time_date() == variant.image_time)
        {
            return &variant;
        }
    }
    return nullptr;
}

const XbeVariant* PrimaryVariant()
{
    return &kVariants[0];
}

size_t BindTitleHle(const rexbox::xbe::XbeImage& image, rexbox::runtime::HleRegistry& registry)
{
    // The variable addresses were identified in the primary variant's XBE.
    const RexboxXdkVariable* variables = RecompiledXdkVariables();
    if (variables == nullptr || IdentifyXbe(image) != PrimaryVariant())
    {
        return 0;
    }
    size_t count = 0;
    for (; variables[count].name != nullptr; ++count)
    {
        registry.BindVariable(variables[count].name, variables[count].address);
    }
    for (const auto& internal : kXdkInternals)
    {
        registry.BindVariable(internal.name, internal.address);
        ++count;
    }
    return count;
}

}  // namespace reexceed
