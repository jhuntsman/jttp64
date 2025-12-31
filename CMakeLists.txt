cmake_minimum_required(VERSION 3.10)

# Disable compiler identification probes
set(CMAKE_C_COMPILER_ID_RUN TRUE)
set(CMAKE_C_COMPILER_FORCED TRUE)
set(CMAKE_C_COMPILER_WORKS TRUE)

project(jttp64 C)

# Ensure CMake doesn't try to add standard flags like --coverage or -fPIC
set(CMAKE_C_INITIAL_FLAGS "")
set(CMAKE_EXE_LINKER_FLAGS "")

# Configuration Paths
set(OSCAR_BIN "/opt/oscar64/bin/oscar64")
set(PRG_NAME "JTTP.PRG")
set(DBG_NAME "JTTP.DBJ")
set(OUTPUT_PRG "${CMAKE_CURRENT_BINARY_DIR}/${PRG_NAME}")
set(OUTPUT_DBG "${CMAKE_CURRENT_BINARY_DIR}/${DBG_NAME}")

# Collect ALL files for CLion's Intelligence
file(GLOB_RECURSE SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c")
file(GLOB_RECURSE HEADER_FILES "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h" "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h")
file(GLOB_RECURSE ASSET_FILES "${CMAKE_CURRENT_SOURCE_DIR}/assets/*.bin")

# Tell CMake to copy your assets to the build folder so #embed can find them
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/assets"
        DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")

# Oscar64 Flags & Includes
set(OSCAR_FLAGS "-n" "-g" "-tf=prg" "-O0")
set(OSCAR_DEFS "-dDEBUG" "-d__oscar64__")
set(OSCAR_INCLUDES
        "-i=${CMAKE_CURRENT_SOURCE_DIR}/src"
        "-i=${CMAKE_CURRENT_SOURCE_DIR}/include"
        "-i=${CMAKE_CURRENT_SOURCE_DIR}/assets"
        "-i=/opt/oscar64/include"
)

# The Build Rule
# We use the full path in OUTPUT so Ninja knows exactly where to look
add_custom_command(
        OUTPUT "${OUTPUT_PRG}" "${OUTPUT_DBG}"
        COMMAND ${OSCAR_BIN} "-o=${OUTPUT_PRG}" ${OSCAR_FLAGS} ${OSCAR_DEFS} ${OSCAR_INCLUDES} ${SOURCE_FILES}
        DEPENDS ${SOURCE_FILES} ${HEADER_FILES} ${ASSET_FILES}
        COMMENT "Compiling for C64: ${PRG_NAME}"
        VERBATIM
)

# The Build Target (What you click 'Build' on)
add_custom_target(jttp64 ALL DEPENDS ${OUTPUT_PRG})

# THE FIX FOR CLION: The Dummy Target
# This tells CLion: "These files belong to a target, please index them!"
# EXCLUDE_FROM_ALL ensures CMake doesn't actually try to compile this with a host compiler.
add_executable(clion_sync EXCLUDE_FROM_ALL ${SOURCE_FILES} ${HEADER_FILES}
        src/globals.c
        src/assets.c)
target_include_directories(clion_sync PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/src"
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/assets"
        "/opt/oscar64/include"
)

# Manually define macros for the IDE so it understands Oscar64-specific code
target_compile_definitions(clion_sync PRIVATE
        DEBUG=1
        __oscar64__=1
)