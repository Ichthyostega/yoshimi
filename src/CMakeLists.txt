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

# WARNING: this is a Stripped-down variant -- use only for Bug Demo / Wayland problem 12/2025


cmake_minimum_required (VERSION 3.12)

set (CMAKE_CXX_STANDARD 17) # we seem to need both for
add_definitions(-std=gnu++17) # various versions of cmake

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -g3 -ggdb")

# avoid exporting any (spurious) symbols in EXE and Plugin
# Note: the two LV2 descriptors are already marked visibility=default in LV2 headers
cmake_policy (SET CMP0063 NEW)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN 1)



project (Yoshimi)

set (YOSHIMI_VERSION "2.3.5.3")

set (CMAKE_INCLUDE_CURRENT_DIR ON)

set (CMAKE_SKIP_RULE_DEPENDENCY OFF)
    # Rebuild objects if rules have changed, even if source hasn't.

message (STATUS "Building Yoshimi Wayland-Bug-Test (v${YOSHIMI_VERSION}) for ${CMAKE_SYSTEM_NAME}")

include (CheckCSourceCompiles)
include(GNUInstallDirs)


find_package (PkgConfig REQUIRED)


# libcairo
pkg_check_modules (LIBCAIRO REQUIRED cairo)

# fltk
find_package (FLTK REQUIRED)
mark_as_advanced (FLTK_DIR)
mark_as_advanced (FLTK_FLUID_EXECUTABLE)
mark_as_advanced (FLTK_MATH_LIBRARY)




file(GLOB_RECURSE DSP_sources CONFIGURE_DEPENDS "DSP/*.cpp")

file(GLOB_RECURSE Effects_sources CONFIGURE_DEPENDS "Effects/*.cpp")

file(GLOB_RECURSE Misc_sources CONFIGURE_DEPENDS "Misc/*.cpp")

file(GLOB_RECURSE Interface_Sources CONFIGURE_DEPENDS "Interface/*.cpp")

file(GLOB_RECURSE CLI_Sources CONFIGURE_DEPENDS "CLI/*.cpp")

file(GLOB_RECURSE Params_sources CONFIGURE_DEPENDS "Params/*.cpp")

file(GLOB_RECURSE Synth_sources CONFIGURE_DEPENDS "Synth/*.cpp")

file(GLOB_RECURSE MusicIO_sources CONFIGURE_DEPENDS "MusicIO/*.cpp")

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

add_definitions (
    -D'YOSHIMI_VERSION="${YOSHIMI_VERSION}"'
)

add_definitions (-DYOSHIMI="yoshimi")



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
    ${FLTK_INCLUDE_DIR}
    ${LIBCAIRO_INCLUDE_DIRS}
)

set(ExternLibraries
    ${FLTK_LIBRARIES}
    ${LIBCAIRO_LIBRARIES}
)


add_executable (yoshimi ${ProgSources} main.cpp)

target_link_libraries (yoshimi ${ExternLibraries})
target_link_libraries (yoshimi stdc++fs)


set_directory_properties (PROPERTIES
    ADDITIONAL_MAKE_CLEAN_FILES "${FltkUI_headers}"
)
