# SPDX-FileCopyrightText: (C) 2012 - 2026 Synergy App Ltd
# SPDX-License-Identifier: MIT

# Must be included after deskflow's project() call and the CMAKE_PROJECT_*
# defaults are set, so these overrides take effect.
set(CMAKE_PROJECT_PROPER_NAME "Synergy")
set(CMAKE_PROJECT_VENDOR "Synergy App Ltd")
set(CMAKE_PROJECT_COPYRIGHT "(C) 2012-2026 ${CMAKE_PROJECT_VENDOR}")
set(CMAKE_PROJECT_CONTACT "${CMAKE_PROJECT_PROPER_NAME} <support@synergyapp.io>")
set(CMAKE_PROJECT_REV_FQDN "com.symless.synergy")
set(CMAKE_PROJECT_DOMAIN "synergyapp.io")
set(CMAKE_PROJECT_HOMEPAGE_URL "https://synergyapp.io")

# Display brand. "Synergy 1" is the default user-facing name (window title,
# About dialog). When building as the Core, flip to "Synergy Core" so the 
# same codebase ships under a different product label.
# Distinct from CMAKE_PROJECT_PROPER_NAME, which stays "Synergy" to keep file paths
# (~/.config/Synergy/, Synergy.conf) and Windows globals space-free.
option(SYNERGY_CORE_FLAVOR "Build as Synergy Core" OFF)
if(SYNERGY_CORE_FLAVOR)
  set(SYNERGY_DISPLAY_NAME "Synergy Core")
else()
  set(SYNERGY_DISPLAY_NAME "Synergy 1")
endif()
add_compile_definitions(SYNERGY_DISPLAY_NAME="${SYNERGY_DISPLAY_NAME}")

# Core flavor seeds headless-build defaults (no GUI, no tests, no installer).
# No `FORCE` on the cache writes: the seed only fills empty slots, so a user
# passing -DBUILD_GUI=ON alongside the flavor flag still wins.
if(SYNERGY_CORE_FLAVOR)
  set(BUILD_GUI OFF CACHE BOOL "Build GUI")
  set(BUILD_TESTS OFF CACHE BOOL "Build tests")
  set(BUILD_INSTALLER OFF CACHE BOOL "Build installer")
endif()

# Don't run unit tests as part of the build. Devs can opt back in with
# -DSKIP_BUILD_TESTS=OFF if they want post-build ctest invocation.
set(SKIP_BUILD_TESTS ON CACHE BOOL "Skip build time test")

# Resource paths consumed by extra/src/lib/synergy/gui/CMakeLists.txt.
set(GUI_RES_DIR "${CMAKE_SOURCE_DIR}/extra/src/apps/res")
set(GUI_QRC_FILE "${GUI_RES_DIR}/synergy.qrc")

# Override deskflow's project name. This cascades into binary names
# (${CMAKE_PROJECT_NAME}-core, etc.), install paths, package names,
# translation file naming, and CPack metadata. Source files in src/apps/*/
# are patched to use literal filenames since they previously assumed
# target name == source basename.
set(CMAKE_PROJECT_NAME synergy)

# Synergy version mode suffix. Base version (MAJOR.MINOR.PATCH) is set in
# the root CMakeLists.txt. Default mode is dev; flip with -DSYNERGY_VERSION_RELEASE=ON
# or -DSYNERGY_VERSION_SNAPSHOT=ON for CI/release builds.
option(SYNERGY_VERSION_RELEASE "Release version" OFF)
option(SYNERGY_VERSION_SNAPSHOT "Snapshot version" OFF)

# Compute revision count once. Used as both the +rN suffix in snapshot version
# strings and as CMAKE_PROJECT_VERSION_TWEAK (consumed by VersionInfo.h.in's
# kDisplayVersion ternary and src/apps/res/windows.rc.in's VER_VERSION 4th
# digit, which Windows update mechanisms key off for installer recognition).
set(_rev_count 0)
if(GIT_FOUND)
  execute_process(
    COMMAND ${GIT_EXECUTABLE} describe HEAD --tags --long --match "v[0-9]*"
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    OUTPUT_VARIABLE _git_describe
    ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE
  )
  if(_git_describe MATCHES "-([0-9]+)-g")
    set(_rev_count "${CMAKE_MATCH_1}")
  endif()
endif()

set(_base "${CMAKE_PROJECT_VERSION_MAJOR}.${CMAKE_PROJECT_VERSION_MINOR}.${CMAKE_PROJECT_VERSION_PATCH}")
if(SYNERGY_VERSION_RELEASE)
  set(CMAKE_PROJECT_VERSION "${_base}")
  set(CMAKE_PROJECT_VERSION_TWEAK 0)
elseif(SYNERGY_VERSION_SNAPSHOT)
  set(CMAKE_PROJECT_VERSION "${_base}-snapshot+r${_rev_count}")
  set(CMAKE_PROJECT_VERSION_TWEAK ${_rev_count})
else()
  set(CMAKE_PROJECT_VERSION "${_base}-dev")
  set(CMAKE_PROJECT_VERSION_TWEAK ${_rev_count})
  add_compile_definitions(SYNERGY_VERSION_DEV)
endif()
unset(_base)
unset(_git_describe)
unset(_rev_count)
