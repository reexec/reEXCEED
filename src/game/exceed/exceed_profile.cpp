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
// library (reCT3 binds "D3D_InitializeDeviceState" and "D3D_g_Stream" this
// way). None are known for this build yet; they are added once the lifted
// D3D8 library has been read (docs/analysis/exceed-xbe-overview.md).
constexpr std::array<XdkInternal, 0> kXdkInternals = {};

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
