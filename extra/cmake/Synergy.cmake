# SPDX-FileCopyrightText: (C) 2012 - 2026 Symless Ltd
# SPDX-License-Identifier: MIT

# Must be included after deskflow's project() call and the CMAKE_PROJECT_*
# defaults are set, so these overrides take effect.
set(CMAKE_PROJECT_PROPER_NAME "Synergy")
set(CMAKE_PROJECT_VENDOR "Symless Ltd")
set(CMAKE_PROJECT_COPYRIGHT "(C) 2012-2026 ${CMAKE_PROJECT_VENDOR}")
set(CMAKE_PROJECT_CONTACT "${CMAKE_PROJECT_PROPER_NAME} <support@symless.com>")
set(CMAKE_PROJECT_REV_FQDN "com.symless.synergy")
set(CMAKE_PROJECT_DOMAIN "symless.com")
set(CMAKE_PROJECT_HOMEPAGE_URL "https://symless.com/synergy")

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

set(_base "${CMAKE_PROJECT_VERSION_MAJOR}.${CMAKE_PROJECT_VERSION_MINOR}.${CMAKE_PROJECT_VERSION_PATCH}")
if(SYNERGY_VERSION_RELEASE)
  set(CMAKE_PROJECT_VERSION "${_base}")
  set(CMAKE_PROJECT_VERSION_TWEAK 0)
elseif(SYNERGY_VERSION_SNAPSHOT)
  set(CMAKE_PROJECT_VERSION "${_base}-snapshot")
  set(CMAKE_PROJECT_VERSION_TWEAK 1)
else()
  set(CMAKE_PROJECT_VERSION "${_base}-dev")
  set(CMAKE_PROJECT_VERSION_TWEAK 1)
endif()
unset(_base)
