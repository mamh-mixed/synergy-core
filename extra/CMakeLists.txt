# SPDX-FileCopyrightText: (C) 2012 - 2026 Symless Ltd
# SPDX-License-Identifier: MIT

# Make synergy/... headers under this tree resolvable from anywhere that descends
# from this directory (and from upstream code once hooks are wired).
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/src/lib")

add_subdirectory(src)
