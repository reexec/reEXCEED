# Title-specific tools. Each needs the generated code, so this directory is
# added only when reexceed_recomp exists (scripts/recomp/regenerate.py has run).
#
#   decomp_check/   reexceed_decomp_check: every decompiled function against the
#                   lifted body it replaced (src/game/exceed/decomp/)

add_executable(reexceed_decomp_check decomp_check/main.cpp decomp_check/guest_fpu.c)
target_link_libraries(reexceed_decomp_check PRIVATE reexceed_game)
# guest_fpu.c reads the x87 and SEH state through the generated code's header.
target_include_directories(reexceed_decomp_check PRIVATE "${REEXCEED_RECOMP_GEN_DIR}")
rexbox_strict_warnings(reexceed_decomp_check)
if(MSVC)
    # The lifter's header's constant casts (C4310), as for the decompiled sources.
    set_source_files_properties(decomp_check/guest_fpu.c PROPERTIES COMPILE_OPTIONS /wd4310)
endif()
add_test(NAME reexceed_decomp_check COMMAND reexceed_decomp_check)
