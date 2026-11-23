// reexceed_settings_probe: verifies reexceed.ini round-trip serialization for
// PostShader and the CLI > environment > ini > none precedence (issue #20,
// docs/design/20261124-i020-screen-shaders.md).

#include "settings.h"

#include "rexbox/hle/d3d8/d3d8_post_shader.h"

#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <string>
#include <system_error>

#define PROBE_ASSERT(cond)                                                               \
    do                                                                                   \
    {                                                                                    \
        if (!(cond))                                                                     \
        {                                                                                \
            std::fprintf(stderr, "PROBE_ASSERT failed at %s:%d: %s\n",                   \
                         __FILE__, __LINE__, #cond);                                     \
            std::fflush(stderr);                                                         \
            std::exit(1);                                                                \
        }                                                                                \
    } while (0)

int main()
{
    std::error_code ec;
    const auto base = std::filesystem::temp_directory_path(ec) / "reexceed_settings_probe";
    std::filesystem::remove_all(base, ec);
    std::filesystem::create_directories(base, ec);
    const std::string ini_path = (base / "reexceed.ini").string();

    // 1. Missing file uses defaults ("none").
    {
        reexceed::SettingsFile file(ini_path);
        const reexceed::Settings loaded = file.Load();
        PROBE_ASSERT(loaded.post_shader == "none");
    }

    // 2. Round-trip save and load preserves PostShader (including case for file names).
    {
        reexceed::SettingsFile file(ini_path);
        reexceed::Settings saved;
        saved.post_shader = "MyShader.glslp";
        PROBE_ASSERT(file.Save(saved));

        const reexceed::Settings loaded = file.Load();
        PROBE_ASSERT(loaded.post_shader == "MyShader.glslp");
    }

    // 3. Empty PostShader in ini falls back to default "none".
    {
        std::ofstream out(ini_path, std::ios::binary | std::ios::trunc);
        out << "[Graphics]\nPostShader = \n";
        out.close();

        reexceed::SettingsFile file(ini_path);
        const reexceed::Settings loaded = file.Load();
        PROBE_ASSERT(loaded.post_shader == "none");
    }

    // 4. Precedence: CLI > environment > ini > none.
    {
        const std::string cli = "fsr";
        const std::string ini = "crt";
        const std::string empty;
        PROBE_ASSERT(rexbox::hle::d3d8::ChoosePostShader(&cli, "scanline", &ini) == "fsr");
        PROBE_ASSERT(rexbox::hle::d3d8::ChoosePostShader(nullptr, "scanline", &ini) == "scanline");
        PROBE_ASSERT(rexbox::hle::d3d8::ChoosePostShader(&empty, "scanline", &ini) == "scanline");
        PROBE_ASSERT(rexbox::hle::d3d8::ChoosePostShader(nullptr, "", &ini) == "crt");
        PROBE_ASSERT(rexbox::hle::d3d8::ChoosePostShader(nullptr, nullptr, &ini) == "crt");
        PROBE_ASSERT(rexbox::hle::d3d8::ChoosePostShader(nullptr, nullptr, &empty) == "none");
        PROBE_ASSERT(rexbox::hle::d3d8::ChoosePostShader(nullptr, nullptr, nullptr) == "none");
    }

    std::filesystem::remove_all(base, ec);
    std::printf("reexceed_settings_probe: PASS\n");
    return 0;
}
