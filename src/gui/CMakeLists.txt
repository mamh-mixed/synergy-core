cmake_minimum_required (VERSION 3.4)

find_package (Qt5 COMPONENTS Core Widgets Network)
set (CMAKE_AUTOMOC ON)
set (CMAKE_AUTORCC ON)
set (CMAKE_AUTOUIC ON)
set (CMAKE_INCLUDE_CURRENT_DIR ON)

file (GLOB LEGACY_GUI_SOURCE_FILES src/*.cpp src/*.h)
file (GLOB LEGACY_GUI_UI_FILES res/*.ui)

add_executable (synergy
    ${LEGACY_GUI_SOURCE_FILES}
    ${LEGACY_GUI_UI_FILES}
    res/Synergy.qrc
)

include_directories (../lib/shared ./src)

if (WIN32)
    include_directories ($ENV{BONJOUR_SDK_HOME}/Include)
    find_library (DNSSD_LIB dnssd.lib 
                  HINTS ENV BONJOUR_SDK_HOME
                  PATH_SUFFIXES "Lib/x64")
endif()

qt5_use_modules (synergy Core Widgets Network)
target_link_libraries (synergy shared)
target_compile_definitions (synergy PRIVATE -DVERSION_STAGE="${VERSION_STAGE}")
target_compile_definitions (synergy PRIVATE -DVERSION_REVISION="")

if (WIN32)
	target_link_libraries (synergy ${DNSSD_LIB})
    set_target_properties (synergy PROPERTIES LINK_FLAGS "/NODEFAULTLIB:LIBCMT")
endif()