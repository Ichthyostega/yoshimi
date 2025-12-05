#
#   CMakeLists.txt
#
#   Copyright 2009-2011, Alan Calvert
#   Copyright 2014-2023, Will Godfrey & others
#   Copyright 2024, Kristian Amlie, Will Godfrey
#
#   This file is part of yoshimi, which is free software: you can
#   redistribute it and/or modify it under the terms of the GNU General
#   Public License as published by the Free Software Foundation, either
#   version 2 of the License, or (at your option) any later version.
#
#   yoshimi is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with yoshimi.  If not, see <http://www.gnu.org/licenses/>.

cmake_minimum_required (VERSION 3.12)
cmake_policy (VERSION 3.12)
cmake_policy (SET CMP0004 NEW)
if (POLICY CMP0046)
    cmake_policy (SET CMP0046 NEW)
endif () # opengl version
if (POLICY CMP0072)
    cmake_policy (SET CMP0072 NEW)
endif ()

#set(CMAKE_CXX_COMPILER "clang++")

set (CMAKE_CXX_STANDARD 17) # we seem to need both for
add_definitions(-std=gnu++17) # various versions of cmake

add_definitions(-Wall)

# avoid exporting any (spurious) symbols in EXE and Plugin
# Note: the two LV2 descriptors are already marked visibility=default in LV2 headers
cmake_policy (SET CMP0063 NEW)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN 1)

# vvv
#add_definitions(-Wpedantic) # full ISO spec
#add_definitions(-Werror) # warnings as errors
#add_definitions(-Wfatal-errors) # stop on first
add_definitions(-Werror=format-security)
#add_definitions(-fsanitize=address)
add_definitions(-Wextra)
add_definitions(-Wno-vla) # Nothing wrong with variable length arrays
#add_definitions(-Wno-unqualified-std-cast-call) # supresses clang warning 'unqualified call'
# ^^^ comment these out on release pushes


project (Yoshimi)

set (YOSHIMI_VERSION "2.3.5.3")
file (WRITE version.txt "${YOSHIMI_VERSION}")


set (CMAKE_INCLUDE_CURRENT_DIR ON)
set (CMAKE_USE_RELATIVE_PATHS OFF)
    # relative paths break some scripts(?)
set (CMAKE_SKIP_RULE_DEPENDENCY OFF)
    # Rebuild objects if rules have changed, even if source hasn't.

message (STATUS "Building Yoshimi version ${YOSHIMI_VERSION} for ${CMAKE_SYSTEM_NAME}")

include (CheckCSourceCompiles)
include(GNUInstallDirs)

mark_as_advanced (EXECUTABLE_OUTPUT_PATH)
mark_as_advanced (LIBRARY_OUTPUT_PATH)
mark_as_advanced (CMAKE_BUILD_TYPE)
mark_as_advanced (CMAKE_INSTALL_PREFIX)

# Check for jack session availability
check_c_source_compiles (
    "#include <jack/session.h>
    int main(int argc, char **argv)
    {
        jack_session_event_type_t x = JackSessionSave;
        return 0;
    }" HasJackSession
)

if (HasJackSession)
    option (JackSessionSupport "Include Jack Session Support" ON)
else()
    set (JackSessionSupport OFF)
endif ()

option (BuildFor0ld_X86 "Build for legacy x86 system" OFF)

option (BuildForAMD_X86_64 "Build for AMD x86_64 system" OFF)
option (BuildForCore2_X86_64 "Build for Intel Core2 x86_64 system" OFF)
option (BuildForRasPi4 "Build for ARM Cortex A9 system" OFF)
option (BuildForThisCPU "Build for presently running system" OFF)
option (BuildForDebug "Debug build without optimisation. Enable gdb debugging support" OFF)
option (BuildForDiagnostic "Add diagnostics and debug info for optimised release build" OFF)

option (BuildWithFLTK "Build with FLTK GUI" ON)

# developer info
option (ShowNoteOnTimes "Report Note-On Times" OFF)
option (ReportNoteEvents "Report Notes On/Off Count" OFF)


set (BuildOptions_X86
    "-msse -msse2 -mfpmath=sse -ffast-math -fomit-frame-pointer"
    CACHE STRING "basic X86 compiler options"
)

set (BuildOptions_X86_64AMD
    "-march=athlon64 -m64 -Wall -msse -msse2 -mfpmath=sse -ffast-math -fno-finite-math-only -fomit-frame-pointer"
    CACHE STRING "X86_64 compiler options"
)

set (BuildOptions_X86_64Core2
    "-march=core2 -m64 -Wall -msse -msse2 -mfpmath=sse -ffast-math -fno-finite-math-only -fomit-frame-pointer"
    CACHE STRING "X86_64 compiler options"
)

set (BuildOptions_RasPi4
    "-march=native -mfloat-abi=hard -mfpu=vfp -mcpu=cortex-a72 -mtune=cortex-a72 -pipe -mvectorize-with-neon-quad -funsafe-loop-optimizations"
    CACHE STRING "Raspberry Pi V4 compiler options"
)

set (BuildOptions_ThisCPU
    "-march=native -mtune=native -Wall"
    CACHE STRING "Current CPU compiler options"
)

set (BuildOptions_Basic
    "-ffast-math -fomit-frame-pointer"
    CACHE STRING "basic compiler options"
)

set (BuildOptions_Debug
    "-O0 -Wpointer-arith -Wpedantic -Wextra" CACHE STRING "Compile without optimisation and with debug information")
set (BuildOptions_Diagnostic
    "-g3 -ggdb" CACHE STRING "Additional flags for debug information and diagnostics")
set (BuildOptions_Release
    "-O3" CACHE STRING "Compile with optimisation for Release builds")

if (BuildWithFLTK)
    add_definitions (-DGUI_FLTK)
endif ()

find_package (PkgConfig REQUIRED)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

check_c_source_compiles (
    "#include <argp.h>
    #include <stdlib.h>
    int main () {
        int argc=1;
        char *argv[]={\"test\"};
        argp_parse(0,argc,argv,0,0,0);
        return 0;
    }" LIBC_HAS_ARGP
)

if (NOT LIBC_HAS_ARGP)
    message(STATUS "libc does not have argp")
    find_library (ARGP_LIB argp REQUIRED)
endif ()

# libz
find_package(ZLIB REQUIRED)

# fftw3f
pkg_check_modules (FFTW3F REQUIRED fftw3f>=0.22)

# mxml
pkg_search_module (MXML REQUIRED mxml4 mxml>=2.5)

# Alsa
if ("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
    pkg_check_modules (ALSA REQUIRED alsa>=1.0.17)
    add_definitions(-DHAVE_ALSA)
    FIND_LIBRARY(LIBDL_LINUX NAMES dl)
else()

endif ()

# Jack
pkg_check_modules (JACK REQUIRED jack>=0.115.6)

# fontconfig
pkg_check_modules (FONTCONFIG REQUIRED fontconfig>=0.22)
mark_as_advanced(FONTCONFIG_LIBRARIES)


if (BuildWithFLTK)
# libcairo
    pkg_check_modules (LIBCAIRO REQUIRED cairo)

# fltk
    find_package (FLTK REQUIRED)
    mark_as_advanced (FLTK_DIR)
    mark_as_advanced (FLTK_FLUID_EXECUTABLE)
    mark_as_advanced (FLTK_MATH_LIBRARY)
endif ()


# libncurses / libcursesw
set(CURSES_NEED_NCURSES TRUE)
find_package (Curses REQUIRED)

find_path(Readline_ROOT_DIR
    NAMES include/readline/readline.h
)

find_path(Readline_INCLUDE_DIR
    NAMES readline/readline.h
    HINTS ${Readline_ROOT_DIR}/include
)

find_library(Readline_LIBRARY
    NAMES readline
    HINTS ${Readline_ROOT_DIR}/lib
)

#find_package(PkgConfig REQUIRED)
if (Readline_INCLUDE_DIR AND Readline_LIBRARY AND CURSES_LIBRARY)
    set(READLINE_FOUND TRUE)
else()
    FIND_LIBRARY(Readline_LIBRARY NAMES readline)
    include(FindPackageHandleStandardArgs)
    FIND_PACKAGE_HANDLE_STANDARD_ARGS(Readline DEFAULT_MSG Readline_INCLUDE_DIR Readline_LIBRARY )
    MARK_AS_ADVANCED(Readline_INCLUDE_DIR Readline_LIBRARY)
endif ()

mark_as_advanced(
    Readline_ROOT_DIR
    Readline_INCLUDE_DIR
    Readline_LIBRARY
)

if (READLINE_FOUND)
    message (STATUS "Readline library found" )
else()
    message( FATAL_ERROR "Readline library not found! Please install development components (libreadline-dev)" )
endif ()

# set platform specific compiler flags
if (BuildFor0ld_X86)
    set (OurCxxFlags ${BuildOptions_X86})
elseif (BuildForAMD_X86_64)
    set (OurCxxFlags ${BuildOptions_X86_64AMD})
elseif (BuildForCore2_X86_64)
    set (OurCxxFlags ${BuildOptions_X86_64Core2})
elseif (BuildForNEON)
    set (OurCxxFlags ${BuildOptions_NEON})
elseif (BuildForThisCPU)
    set (OurCxxFlags ${BuildOptions_ThisCPU})
else()
    set (OurCxxFlags ${BuildOptions_Basic})
endif ()
set (CMAKE_CXX_FLAGS "${OurCxxFlags} ${CMAKE_CXX_FLAGS}")

if (BuildForDiagnostic OR BuildForDebug)
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${BuildOptions_Diagnostic}")
else()
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -DNDEBUG ")
endif ()

if (BuildForDebug)
    set (CMAKE_BUILD_TYPE "Debug")
    set (CMAKE_CXX_FLAGS_DEBUG ${BuildOptions_Debug})
    message (STATUS "DEBUG Build, flags: ${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_DEBUG}")
else()
    set (CMAKE_BUILD_TYPE "Release")
    set (CMAKE_CXX_FLAGS_RELEASE ${BuildOptions_Release})
    message (STATUS "RELEASE Build, flags: ${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_RELEASE}")
endif ()

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/../desktop/yoshimi.desktop.in"
    "${CMAKE_CURRENT_BINARY_DIR}/yoshimi.desktop"
    IMMEDIATE @ONLY)

file(GLOB_RECURSE DSP_sources CONFIGURE_DEPENDS "DSP/*.cpp")

file(GLOB_RECURSE Effects_sources CONFIGURE_DEPENDS "Effects/*.cpp")

file(GLOB_RECURSE Misc_sources CONFIGURE_DEPENDS "Misc/*.cpp")

file(GLOB_RECURSE Interface_Sources CONFIGURE_DEPENDS "Interface/*.cpp")

file(GLOB_RECURSE CLI_Sources CONFIGURE_DEPENDS "CLI/*.cpp")

file(GLOB_RECURSE Params_sources CONFIGURE_DEPENDS "Params/*.cpp")

file(GLOB_RECURSE Synth_sources CONFIGURE_DEPENDS "Synth/*.cpp")

file(GLOB_RECURSE MusicIO_sources CONFIGURE_DEPENDS "MusicIO/*.cpp")

if (BuildWithFLTK)
    file(GLOB_RECURSE FltkUI_specs CONFIGURE_DEPENDS "UI/*.fl")
    file(GLOB_RECURSE FltkUI_additionalSrc CONFIGURE_DEPENDS "UI/*.cpp")

    set (guitmp generated-gui-sources)
    file (MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/${guitmp}")

# workaround fltk_wrap_ui breakage
    set (FltkUI_sources)
    set (FltkUI_headers)
    foreach(filename ${FltkUI_specs})
        get_filename_component(filename "${filename}" NAME)
        string(REGEX REPLACE "\\.[^\\.]+$" "" basename "${filename}") # strip any extension
        set (fluidfile "${CMAKE_CURRENT_SOURCE_DIR}/UI/${basename}.fl")
        set (sourcefile "${basename}.cpp")
        set (headerfile "${basename}.h")
        add_custom_command(
            OUTPUT ${sourcefile}
            WORKING_DIRECTORY ${guitmp}
            COMMAND ${FLTK_FLUID_EXECUTABLE} ARGS -c -o .cpp ${fluidfile}
            COMMAND ${CMAKE_COMMAND} ARGS -E copy_if_different ${sourcefile} ..
            COMMAND ${CMAKE_COMMAND} ARGS -E copy_if_different ${headerfile} ..
            DEPENDS ${fluidfile}
        )
        set (FltkUI_sources ${FltkUI_sources} "${sourcefile}")
        set (FltkUI_headers ${FltkUI_headers} "${headerfile}")
    endforeach(filename ${FltkUI_specs})

    set (FltkUI_sources
        ${FltkUI_sources}
        ${FltkUI_additionalSrc}
    )
    set (YOSHI_INCLUDES ${FLTK_INCLUDE_DIR})
endif ()

add_definitions (
    -D'YOSHIMI_VERSION="${YOSHIMI_VERSION}"'
    -D'BASE_INSTALL_DIR="${CMAKE_INSTALL_PREFIX}"'
    -DYOSHI_FIFO_DIR="${FifoDirectory}"
)

add_definitions (-DYOSHIMI="yoshimi")

if (ShowNoteOnTimes)
    add_definitions (-DREPORT_NOTE_ON_TIME)
endif ()

if (ReportNoteEvents)
    add_definitions (-DREPORT_NOTES_ON_OFF)
endif ()

if (JackSessionSupport)
    message(STATUS "With jack session support")
    add_definitions(-DJACK_SESSION)
else()
    message(STATUS "Without jack session support")
endif ()

# Check for jack latency API >= 0.120.1
check_c_source_compiles (
    "#include <jack/jack.h>
    int main(int argc, char **argv)
    {
        if (jack_set_latency_callback)
            return 0;
        else
            return 1;
    }" HasJackLatency
)

if (HasJackLatency)
    add_definitions(-DJACK_LATENCY)
endif ()

set (ProgSources
    ${Interface_Sources}
    ${CLI_Sources}
    ${Misc_sources}
    ${Params_sources}
    ${Synth_sources}
    ${DSP_sources}
    ${Effects_sources}
    ${MusicIO_sources}
    ${FltkUI_sources}
)

include_directories (AFTER
    ${MXML_INCLUDE_DIRS}
    ${ALSA_INCLUDE_DIRS}
    ${JACK_INCLUDE_DIRS}
    ${FONTCONFIG_INCLUDE_DIRS}
    ${FLTK_INCLUDE_DIR}
    ${FFTW3F_INCLUDE_DIRS}
    ${LIBCAIRO_INCLUDE_DIRS}
    ${Readline_INCLUDE_DIR}
    ${ZLIB_INCLUDE_DIRS}
)

set(ExternLibraries
    ${FONTCONFIG_LIBRARIES}
    ${FLTK_LIBRARIES}
    ${MXML_LIBRARIES}
    ${ALSA_LIBRARIES}
    ${JACK_LINK_LIBRARIES}
    ${LIBSNDFILE_LIBRARIES}
    ${FFTW3F_LIBRARIES}
    ${LIBCAIRO_LIBRARIES}
    ${CURSES_LIBRARIES}
    ${Readline_LIBRARY}
    ${ZLIB_LIBRARIES}
    ${LIBDL_LINUX}
    ${CMAKE_THREAD_LIBS_INIT}
)

if ("${CMAKE_SYSTEM_NAME}" STREQUAL "FreeBSD")
    link_directories(/usr/local/lib)
endif ()

add_executable (yoshimi ${ProgSources} main.cpp)

target_link_libraries (yoshimi ${ExternLibraries})
target_link_libraries (yoshimi stdc++fs)


if (ARGP_LIB)
    target_link_libraries (yoshimi ${ARGP_LIB})
endif ()

if (${CMAKE_SYSTEM_PROCESSOR} STREQUAL "riscv64")
    target_link_libraries (yoshimi atomic)
endif ()

install (TARGETS yoshimi RUNTIME DESTINATION bin)

install (DIRECTORY ../banks DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/yoshimi
    FILE_PERMISSIONS
        OWNER_READ OWNER_WRITE
        GROUP_READ GROUP_WRITE
        WORLD_READ
    DIRECTORY_PERMISSIONS
        OWNER_READ OWNER_WRITE OWNER_EXECUTE
        GROUP_READ GROUP_WRITE GROUP_EXECUTE
        WORLD_READ WORLD_EXECUTE
)

install (DIRECTORY ../doc/examples DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/yoshimi
    FILE_PERMISSIONS
        OWNER_READ OWNER_WRITE
        GROUP_READ GROUP_WRITE
        WORLD_READ
    DIRECTORY_PERMISSIONS
        OWNER_READ OWNER_WRITE OWNER_EXECUTE
        GROUP_READ GROUP_WRITE GROUP_EXECUTE
        WORLD_READ WORLD_EXECUTE
)

install (DIRECTORY ../presets DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/yoshimi
    FILE_PERMISSIONS
        OWNER_READ OWNER_WRITE
        GROUP_READ GROUP_WRITE
        WORLD_READ
    DIRECTORY_PERMISSIONS
        OWNER_READ OWNER_WRITE OWNER_EXECUTE
        GROUP_READ GROUP_WRITE GROUP_EXECUTE
        WORLD_READ WORLD_EXECUTE
)

install (DIRECTORY ../doc/ DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/doc/yoshimi
    FILE_PERMISSIONS
        OWNER_READ OWNER_WRITE
        GROUP_READ GROUP_WRITE
        WORLD_READ
    DIRECTORY_PERMISSIONS
        OWNER_READ OWNER_WRITE OWNER_EXECUTE
        GROUP_READ GROUP_WRITE GROUP_EXECUTE
        WORLD_READ WORLD_EXECUTE
)
install (FILES ${CMAKE_CURRENT_BINARY_DIR}/yoshimi.desktop
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/applications)
install (FILES ${CMAKE_CURRENT_SOURCE_DIR}/../desktop/yoshimi.png
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/pixmaps)
install (FILES ${CMAKE_CURRENT_SOURCE_DIR}/../desktop/64/yoshimi.png
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/64x64/apps)
install (FILES ${CMAKE_CURRENT_SOURCE_DIR}/../desktop/128/yoshimi.png
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/128x128/apps)
install (FILES ${CMAKE_CURRENT_SOURCE_DIR}/../desktop/256/yoshimi.png
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/256x256/apps)
install (FILES ${CMAKE_CURRENT_SOURCE_DIR}/../desktop/yoshimi.svg
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/scalable/apps)
install (FILES ${CMAKE_CURRENT_SOURCE_DIR}/../desktop/yoshimi_alt.svg
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/scalable/apps)
install (FILES ${CMAKE_CURRENT_SOURCE_DIR}/../desktop/metainfo/yoshimi.metainfo.xml
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/metainfo)
install (FILES ${CMAKE_CURRENT_SOURCE_DIR}/../desktop/metainfo/yoshimi.appdata.xml
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/metainfo)
install (FILES ${CMAKE_CURRENT_SOURCE_DIR}/../desktop/yoshimi.1
    DESTINATION ${CMAKE_INSTALL_MANDIR}/man1)

set_directory_properties (PROPERTIES
    ADDITIONAL_MAKE_CLEAN_FILES "${FltkUI_headers}"
)

add_custom_target (showversion
    COMMAND echo -n "Version: "
    COMMAND cat version.txt
    COMMAND echo
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

configure_file(${CMAKE_SOURCE_DIR}/version.txt
            ${CMAKE_SOURCE_DIR}/../doc/yoshimi_user_guide/files/yoshimi_user_guide_version
            COPYONLY
)

# clean sweep
add_custom_target (distclean
    COMMAND rm -f ${CMAKE_SOURCE_DIR}/*.log
    COMMAND rm -f ${CMAKE_SOURCE_DIR}/Makefile
    COMMAND rm -f ${CMAKE_SOURCE_DIR}/install_manifest.txt
    COMMAND rm -f ${CMAKE_SOURCE_DIR}/cmake_install.cmake
    COMMAND find ${CMAKE_SOURCE_DIR} -type f -name CMakeCache.txt | xargs -r rm -f
    COMMAND find ${CMAKE_SOURCE_DIR} -type d -name CMakeFiles | xargs -r rm -rf
    COMMAND find ${CMAKE_SOURCE_DIR} -type f -name "*.marks" | xargs -r rm -f
    COMMAND rm -f ${CMAKE_SOURCE_DIR}/LV2_Plugin/Makefile
    COMMAND rm -f ${CMAKE_SOURCE_DIR}/LV2_Plugin/cmake_install.cmake
    COMMAND rm -f ${CMAKE_SOURCE_DIR}/LV2_Plugin/ -type d -name CMakeFiles | xargs -r rm -rff
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)
