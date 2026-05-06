# SPDX-FileCopyrightText: (C) 2012 - 2026 Symless Ltd
# SPDX-License-Identifier: MIT

# Must be included after deskflow's project() call and the CMAKE_PROJECT_*
# defaults are set, so these overrides take effect.
set(CMAKE_PROJECT_PROPER_NAME "Synergy")
set(CMAKE_PROJECT_VENDOR "Symless Ltd")
set(CMAKE_PROJECT_COPYRIGHT "(C) 2012-2026 ${CMAKE_PROJECT_VENDOR}")
set(CMAKE_PROJECT_CONTACT "${CMAKE_PROJECT_PROPER_NAME} <support@symless.com>")
set(CMAKE_PROJECT_REV_FQDN "com.symless.synergy")
set(CMAKE_PROJECT_HOMEPAGE_URL "https://symless.com/synergy")

# Don't run unit tests as part of the build. Devs can opt back in with
# -DSKIP_BUILD_TESTS=OFF if they want post-build ctest invocation.
set(SKIP_BUILD_TESTS ON CACHE BOOL "Skip build time test")

message(STATUS "Synergy branding applied: ${CMAKE_PROJECT_PROPER_NAME} ${CMAKE_PROJECT_VERSION}")
