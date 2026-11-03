# reexceed_game: everything specific to Pump It Up: Exceed.
#
#   exceed_profile.cpp    identity of supported XBE builds, HLE address bindings
#   exceed_recomp.cpp     the recompiled entry point, when generated code exists
#   exceed_host.c         the host hooks the decompiled sources call (include/reexceed/exceed_host.h)
#   decomp/            hand-decompiled functions that replace lifted ones
#   recomp/            the recompilation configuration (variants, seeds, HLE libraries)
#   <gen dir>          lifted C from scripts/recomp/regenerate.py (gitignored)
file(GLOB REEXCEED_DECOMP_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/decomp/*.c")

add_library(reexceed_game STATIC
    exceed_profile.cpp
    exceed_recomp.cpp
    exceed_host.c
    ${REEXCEED_DECOMP_SOURCES}
)
target_include_directories(reexceed_game PUBLIC "${PROJECT_SOURCE_DIR}/include")
target_link_libraries(reexceed_game PUBLIC rexbox_recomp)

# The generated code, when scripts/recomp/regenerate.py has run (rexbox's
# cmake/rexbox_recomp.cmake: ABI header check, warnings off, the cycle with
# reexceed_game declared both ways).
rexbox_add_recompiled_library(reexceed_recomp GEN_DIR "${REEXCEED_RECOMP_GEN_DIR}" TITLE reexceed_game)
if(TARGET reexceed_recomp)
    # Decompiled sources include the generated declarations.
    target_include_directories(reexceed_game PRIVATE "${REEXCEED_RECOMP_GEN_DIR}")
    target_compile_definitions(reexceed_game PRIVATE REEXCEED_HAS_RECOMP=1)
    if(MSVC AND REEXCEED_DECOMP_SOURCES)
        # The lifter's header as its authors wrote it: its constant casts
        # (C4310, upstream xboxrecomp v0.16.0's x87 helpers) are not ours to
        # change; rexbox's ABI object turns the same warning off.
        set_source_files_properties(${REEXCEED_DECOMP_SOURCES} PROPERTIES COMPILE_OPTIONS /wd4310)
    endif()
elseif(REEXCEED_DECOMP_SOURCES)
    # Decompiled functions only make sense next to the code they replace.
    set_source_files_properties(${REEXCEED_DECOMP_SOURCES} PROPERTIES HEADER_FILE_ONLY ON)
endif()
