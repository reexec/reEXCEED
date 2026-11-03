cmake_minimum_required(VERSION 3.20)

project(reEXCEED LANGUAGES C CXX)

set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/VERSION"
)
file(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/VERSION" REEXCEED_VERSION LIMIT_COUNT 1)
string(STRIP "${REEXCEED_VERSION}" REEXCEED_VERSION)
if(NOT REEXCEED_VERSION MATCHES "^[0-9]+\\.[0-9]+\\.[0-9]+$")
    message(FATAL_ERROR "VERSION must use major.minor.patch format")
endif()
message(STATUS "reEXCEED version ${REEXCEED_VERSION}")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# The shared Xbox runtime (third_party/rexbox, submodule): platform layer,
# XBE, runtime, lifter ABI adapter, HLE modules, tools and probes. Its
# consumer contract is in its ARCHITECTURE.md section 2.
if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/third_party/rexbox/CMakeLists.txt")
    message(FATAL_ERROR "third_party/rexbox is missing; run: git submodule update --init --recursive")
endif()
# The lifter is this project's: it pins upstream xboxrecomp itself
# (third_party/xboxrecomp), and scripts/recomp/lifter.py runs it. The generated
# code carries the lifter's ABI header (gen/<variant>/gen/recomp_types.h), which
# rexbox_add_recompiled_library compiles rexbox's side of the ABI against.
if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/third_party/xboxrecomp/templates/runtime/recomp_types.h")
    message(FATAL_ERROR "third_party/xboxrecomp is missing; run: git submodule update --init --recursive")
endif()
# rexbox_xdk_symbols (for scripts/recomp/regenerate.py) and the probes (CI).
set(REXBOX_BUILD_TOOLS ON CACHE BOOL "Build rexbox_xdk_symbols and the HLE probes")
add_subdirectory(third_party/rexbox)
# The compiler settings every target of the program shares (/EHa above all).
rexbox_apply_host_flags()
message(STATUS "reEXCEED platform: ${REXBOX_PLATFORM}/${REXBOX_ARCH}")

# Lifted C for the primary variant, produced by scripts/recomp/regenerate.py.
# Without it the game links no recompiled code and the app stops after load.
set(REEXCEED_RECOMP_GEN_DIR "${CMAKE_CURRENT_SOURCE_DIR}/gen/exceed_us/gen" CACHE PATH
    "Directory of generated recompiled C (recomp_dispatch.c and friends)")

# Tests: rexbox's probes (registered by the submodule) and the app's smoke
# checks (src/app); `ctest -C <config>` runs them from the build tree, and CI
# runs them on every push (docs/design/20261103-i002-github-actions.md).
enable_testing()

# Layers above rexbox. Each links only the layers below it.
add_subdirectory(src/game/exceed)       # reexceed_game: Exceed title code + profile
add_subdirectory(src/app)            # reexceed: the executable
# reexceed_decomp_check and other tools that need the generated code.
if(TARGET reexceed_recomp)
    add_subdirectory(tools)
endif()

foreach(target IN ITEMS reexceed_game reexceed)
    rexbox_strict_warnings(${target})
endforeach()
