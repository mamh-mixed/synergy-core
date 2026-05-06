# SPDX-FileCopyrightText: (C) 2024 - 2025 Chris Rizzitello <sithlord48@gmail.com>
# SPDX-License-Identifier: MIT

string(TOLOWER "${CMAKE_PROJECT_PROPER_NAME}" _basename)
set(CORE_BINARY "${_basename}-core")
if(WIN32)
  string(APPEND CORE_BINARY ".exe")
endif()
unset(_basename)

configure_file(Constants.h.in Constants.h @ONLY)
configure_file(VersionInfo.h.in VersionInfo.h @ONLY)
configure_file(UrlConstants.h.in UrlConstants.h @ONLY)

add_library(common STATIC
  Coordinate.h
  Enums.h
  ExitCodes.h
  I18N.h
  I18N.cpp
  LogLevel.h
  NetworkProtocol.h
  PlatformInfo.h
  Settings.h
  Settings.cpp
  QSettingsProxy.cpp
  QSettingsProxy.h
  ${CMAKE_CURRENT_BINARY_DIR}/Constants.h
  ${CMAKE_CURRENT_BINARY_DIR}/UrlConstants.h
  ${CMAKE_CURRENT_BINARY_DIR}/VersionInfo.h
)

target_link_libraries(common PUBLIC Qt6::Core)

target_include_directories(common
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src/lib>
    $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/src/lib>
)
