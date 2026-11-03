add_executable(reexceed main.cpp settings.cpp settings_ui.cpp)
target_link_libraries(reexceed PRIVATE reexceed_game rexbox_hle rexbox_platform imgui)
target_compile_definitions(reexceed PRIVATE REEXCEED_VERSION="${REEXCEED_VERSION}" REEXCEED_BUILD_CONFIG="$<CONFIG>")

# The build time in the window title, refreshed on every build.
set(REEXCEED_BUILD_STAMP "${CMAKE_CURRENT_BINARY_DIR}/generated/reexceed_build_stamp.h")
add_custom_target(reexceed_build_stamp
    COMMAND "${CMAKE_COMMAND}" "-DOUTPUT=${REEXCEED_BUILD_STAMP}" "-DSOURCE_DIR=${PROJECT_SOURCE_DIR}"
            -P "${CMAKE_CURRENT_SOURCE_DIR}/build_stamp.cmake"
    BYPRODUCTS "${REEXCEED_BUILD_STAMP}"
    VERBATIM)
add_dependencies(reexceed reexceed_build_stamp)
target_include_directories(reexceed PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/generated")
# Smoke checks that need no game data: the app refuses a file and a directory
# without an XBE with the expected message (its exit code is 1 in both cases).
add_test(NAME reexceed_rejects_file COMMAND reexceed "${PROJECT_SOURCE_DIR}/LICENSE")
set_tests_properties(reexceed_rejects_file PROPERTIES PASS_REGULAR_EXPRESSION "is not a directory")
add_test(NAME reexceed_rejects_directory_without_xbe COMMAND reexceed "${PROJECT_SOURCE_DIR}/docs")
set_tests_properties(reexceed_rejects_directory_without_xbe PROPERTIES PASS_REGULAR_EXPRESSION "has no default.xbe")

if(MSVC)
    # Guest threads get their 64 MiB host stacks explicitly (CreateHostThread);
    # the image default stays small because every other thread in the process
    # (audio pacing, SDL, the GL driver's workers) reserves it too, which
    # exhausts the 32-bit address space (reJSRF issue #25). Large-address-aware
    # gives the process 4 GiB on 64-bit Windows.
    target_link_options(reexceed PRIVATE /LARGEADDRESSAWARE)
    # reexceed.map: function addresses for rexbox's scripts/perf/resolve_samples.py
    # (REXBOX_SAMPLE); the build has no PDB.
    target_link_options(reexceed PRIVATE /MAP)
endif()
