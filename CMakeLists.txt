# QKiSS - A modern implementation of KiSS (Kisekae) viewer in Qt 6.
#
# Copyright (C) 2024 - 2026 ono ono (kuanyui) <azazabc123@gmail.com>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

cmake_minimum_required(VERSION 3.25)  # For Debian 12 (cmake 3.25)

set(PROJ_NAME qkiss) # So this can be used before `project()`
set(PROJ_DISPLAY_NAME QKiSS)
# Read VERSION file to get version
file(READ "${CMAKE_SOURCE_DIR}/VERSION" PROJ_VERSION)
string(STRIP "${PROJ_VERSION}" PROJ_VERSION)

set(PROJ_SUMMARY [[A modern implementation of KiSS (KISekae Set system) viewer in Qt]])
set(PROJ_DESCRIPTION [[QKiSS is a modern implementation of KiSS (Kisekae Set System) viewer in Qt 6, compatible with KISS/GS and FKiSS 1 &amp; 2 specifications, as a complete rebirth inspired by fkiss and GnomeKiSS on Linux.

"Kisekae" comes from the Japanese word "着せ替え人形" (pronunciation: "kisekae ningyou"), meaning "dress-up dolls".

KiSS originated from Japanese otaku subculture and personal computer enthusiasts community (particularly those using the NEC PC-9800 series) in the 1990s, and gained popularity throughout the 1990s and early 2000s.

This project comes from nostalgia for the Windows 95/98 and GTK+ 2 era —— a lost age before Internet was polluted by AI-generated junk (this project does use some AI though). After all, life is frightening enough as it is; the real world is a hot mess, while nijigen and isekai remain a pure promised land.]])

# Descriptions`debian/control`
string(REGEX REPLACE "\n" "\n " PROJ_DESCRIPTION_DEBIAN "${PROJ_DESCRIPTION}")
string(REGEX REPLACE "\n \n" "\n .\n" PROJ_DESCRIPTION_DEBIAN "${PROJ_DESCRIPTION_DEBIAN}")
set(PROJ_DESCRIPTION_DEBIAN "${PROJ_SUMMARY}\n ${PROJ_DESCRIPTION_DEBIAN}")

# ==============================================
# Folder for generated, platform independent files
# ==============================================

# Directory for generated, platform independent files (e.g. XDG icon theme, changelogs...etc)
# No use of `CACHE PATH` here currently.
# A cached variable is defined at first-time `cmake`, and will not be changed no matter how many times `cmake` is executed laterly.
set(PROJ_GENERATED_DIR "${CMAKE_SOURCE_DIR}/generated")

# ==============================================
# Python :: Find Python interprepter for scripts
# ==============================================

# Let user able to explicitly specify `Python3_EXECUTABLE`, because the PATH
# problem under Windows + Python is already a little fucker; when under CI/CD +
# Windows + msvc-dev-cmd + vcpkg + Python + CMake, it immediately transforms
# into a huge pile of ultimate shit.
if (WIN32)
    set(WIN_PYTHON3_EXE "" CACHE FILEPATH "[Windows only] Force specify shitty Python PATH in Windows")
endif()

if (WIN_PYTHON3_EXE)
    file(TO_CMAKE_PATH "${WIN_PYTHON3_EXE}" __PYTHON3__)  # Convert back-slash to slash
    set(__PYTHON3__ "${__PYTHON3__}" CACHE FILEPATH "final python path")  # Use CACHE to remember this value at configure time.
elseif(DEFINED ENV{pythonLocation})
    # If found env var `pythonLocation` is defined (by Github Action `actions/setup-python`)
    file(TO_CMAKE_PATH "$ENV{pythonLocation}/python.exe" __PYTHON3__)  # Convert back-slash to slash
    set(__PYTHON3__ "${__PYTHON3__}" CACHE FILEPATH "final python path")  # Use CACHE to remember this value at configure time.
elseif(Python3_EXECUTABLE)
    file(TO_CMAKE_PATH "${Python3_EXECUTABLE}" __PYTHON3__)  # Convert back-slash to slash
    set(__PYTHON3__ "${__PYTHON3__}" CACHE FILEPATH "final python path")  # Use CACHE to remember this value at configure time.
else()
    find_package(Python3 REQUIRED Interpreter)
    set(__PYTHON3__ "${Python3_EXECUTABLE}")
endif()

message(STATUS "CHECKPOINT: __PYTHON3__ is now set to: ${__PYTHON3__}")

# ===============================================
# Python :: Build XDG-compatible icon theme pack
# ===============================================
set(RESOURCES_DIR "${CMAKE_SOURCE_DIR}/resources")
set(ICON_REPO_DIR "${CMAKE_SOURCE_DIR}/resources/tango-icon-remixer")
set(ICON_BUILD_SCRIPT "${ICON_REPO_DIR}/build_icons.py")
set(ICON_THEME_NAME "CustomizedTango")
set(ICON_PNG_XDG_THEME_DIR "${PROJ_GENERATED_DIR}/icon_theme__png/${ICON_THEME_NAME}")
set(ICON_SVG_XDG_THEME_DIR "${PROJ_GENERATED_DIR}/icon_theme__svg/${ICON_THEME_NAME}")

# When included_icons.txt changed, auto execute cmake again.
set(ICON_INCLUDE_FILE "${RESOURCES_DIR}/included_icons.txt")
set(ICON_GENERATED_STAMP_FILE "${PROJ_GENERATED_DIR}/icons_built.stamp")

# [Windows Only] Formalize path slashes for fucking Windows
foreach(_v RESOURCES_DIR ICON_REPO_DIR ICON_BUILD_SCRIPT ICON_THEME_NAME ICON_PNG_XDG_THEME_DIR ICON_SVG_XDG_THEME_DIR ICON_INCLUDE_FILE ICON_GENERATED_STAMP_FILE)
    file(TO_CMAKE_PATH "${${_v}}" ${_v})
endforeach()

# If qrc does not exist, create empty placeholder
set(ICON_PNG_GENERATED_QRC "${ICON_PNG_XDG_THEME_DIR}/index.qrc")
if(NOT EXISTS ${ICON_PNG_GENERATED_QRC})
    file(WRITE ${ICON_PNG_GENERATED_QRC} "<RCC>\n</RCC>\n")
endif()

# Full script arguments
# --clean-before-build
set(CMD_PYTHON_BUILD_ICONS ${__PYTHON3__} "${ICON_BUILD_SCRIPT}"
    --input-svg-sizes "16,22,24,32,48,256,scalable"
    --output-png-sizes "16,22,24,32,48,256"
    --output-png-sizes-from-scalable "48"
    --included-icon-file "${RESOURCES_DIR}/included_icons.txt"
    --stamp-file "${ICON_GENERATED_STAMP_FILE}"
    --extra-icon-theme "AppIcon:${RESOURCES_DIR}/appicon"
    --output-theme-name "${ICON_THEME_NAME}"
    --output-png-theme-to-dir "${ICON_PNG_XDG_THEME_DIR}"
    --output-svg-theme-to-dir "${ICON_SVG_XDG_THEME_DIR}"
)

# CMake Script Mode (`cmake -P <script_file>`)
# Example: cmake -DTASK=GENERATE_ICONS -DWIN_PYTHON3_EXE=python3 -P CMakeLists.txt
# This is mainly for cache icon result on Github Actions as cache
if(CMAKE_SCRIPT_MODE_FILE)
    if(TASK STREQUAL "GENERATE_ICONS")
        execute_process(COMMAND ${CMD_PYTHON_BUILD_ICONS} RESULT_VARIABLE _icon_rc)
        if(NOT _icon_rc EQUAL 0)
            message(FATAL_ERROR "Icon generation failed")
        endif()
    endif()
    return()  # exit immediately
endif()

# [Configure-time] Ensure icons are all built before `install()`, for packaging (especially some strict build environments like OBS + Fedora).
execute_process(
    COMMAND ${CMD_PYTHON_BUILD_ICONS}
    RESULT_VARIABLE _icon_rc
)
if(NOT _icon_rc EQUAL 0)
    message(FATAL_ERROR "Icon generation failed")
endif()

# [Build-time] Developer convenience (auto execute `build_icons.py` when building project if `included_icons.txt` modified)
add_custom_command(
    OUTPUT ${ICON_GENERATED_STAMP_FILE}
    COMMAND ${CMD_PYTHON_BUILD_ICONS}
    DEPENDS ${ICON_INCLUDE_FILE}  # when this modified, auto run this custom command.
    COMMENT "Generating icon pack..."
    USES_TERMINAL    # immediately output
)
# (custom_target) `icontheme` --needs--> stamp file --generated by--> (custom_command) --needs--> included_icons.txt
add_custom_target(icontheme DEPENDS ${ICON_GENERATED_STAMP_FILE})

# ==============================================
# Python :: Generate changelogs from YAML for RPM & DEB
# ==============================================

execute_process(
    COMMAND ${__PYTHON3__} ${CMAKE_SOURCE_DIR}/scripts/changelog/changelog.py
            --input-yaml ${CMAKE_SOURCE_DIR}/CHANGELOG.yaml
            --output-rpm ${PROJ_GENERATED_DIR}/changelogs_rpm.txt
            --output-deb ${PROJ_GENERATED_DIR}/debian/changelog
            --output-markdown ${PROJ_GENERATED_DIR}/changelogs.md
            --output-latest-markdown ${PROJ_GENERATED_DIR}/latest_changelog.md
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    RESULT_VARIABLE _ret)

if(NOT _ret EQUAL 0)
    message(FATAL_ERROR "Failed to generate changelog")
else()
    message(STATUS "Success to generate changelog.")
endif()

file(READ ${PROJ_GENERATED_DIR}/changelogs_rpm.txt
    CHANGELOG_FOR_RPM_SPEC)

# =======================================================
# Windows .ico file
# =======================================================
if (WIN32)
    # NOTE: `project()` is not executed yet, don't use PROJECT_NAME here.
    find_program(MAGICK_EXECUTABLE NAMES magick convert REQUIRED)
    set(win_ico_names "${PROJ_NAME}" "${PROJ_NAME}-installer" "${PROJ_NAME}-uninstaller")
    set(generated_win_icos)

    foreach(ico_name    IN LISTS    win_ico_names)
        set(SRC_PNGS
            "${ICON_PNG_XDG_THEME_DIR}/16x16/apps/${ico_name}.png"
            "${ICON_PNG_XDG_THEME_DIR}/22x22/apps/${ico_name}.png"
            "${ICON_PNG_XDG_THEME_DIR}/24x24/apps/${ico_name}.png"
            "${ICON_PNG_XDG_THEME_DIR}/32x32/apps/${ico_name}.png"
            "${ICON_PNG_XDG_THEME_DIR}/48x48/apps/${ico_name}.png"
            "${ICON_PNG_XDG_THEME_DIR}/48x48@2/apps/${ico_name}.png"
            "${ICON_PNG_XDG_THEME_DIR}/256x256/apps/${ico_name}.png"
            "${ICON_PNG_XDG_THEME_DIR}/256x256@2/apps/${ico_name}.png"
        )
        set(DST_WIN_ICO "${PROJ_GENERATED_DIR}/windows/${ico_name}.ico")
        # NOTE: Be carefule, this is `add_custom_comamnd()`, not `execute_process()`,
        # so it needs to be bound to a target via `add_custom_target()` or `add_dependencies()`
        add_custom_command(
            OUTPUT ${DST_WIN_ICO}
            COMMAND ${MAGICK_EXECUTABLE} ${SRC_PNGS} ${DST_WIN_ICO}
            DEPENDS ${SRC_PNGS}
            COMMENT "Generating Windows Icon (.ico) from PNG sources"
        )
        list(APPEND generated_win_icos "${DST_WIN_ICO}")
    endforeach()
    # Generate .rc file for main app (not include installer / uninstaller icon)
    set(GEN_WIN_RC_FILE                 "${PROJ_GENERATED_DIR}/windows/${PROJ_NAME}.rc")   # `*.rc` is "Windows Resource", NOT a format defined by Qt.
    set(GEN_WIN_ICO_FILE_MAIN           "${PROJ_GENERATED_DIR}/windows/${PROJ_NAME}.ico")
    set(GEN_WIN_ICO_FILE_INSTALLER      "${PROJ_GENERATED_DIR}/windows/${PROJ_NAME}-installer.ico")
    set(GEN_WIN_ICO_FILE_UNINSTALLER    "${PROJ_GENERATED_DIR}/windows/${PROJ_NAME}-uninstaller.ico")
    file(WRITE "${GEN_WIN_RC_FILE}" "IDI_ICON1 ICON DISCARDABLE \"${GEN_WIN_ICO_FILE_MAIN}\"\n")   # This is file format of Windows Resource
    set(WIN_ICO_RESOURCE "${GEN_WIN_RC_FILE}" "${GEN_WIN_ICO_FILE_MAIN}")  # add to `qt_add_executable()`

    # `ALL` is CMake's "default build target"
    add_custom_target(generate_windows_icons ALL DEPENDS ${generated_win_icos})

endif()


# ==============================================
# Git commit hash / time
# ==============================================
set(GIT_COMMIT_HASH "")
set(GIT_COMMIT_TIME_UNIX_SEC "0")

if(EXISTS "${CMAKE_SOURCE_DIR}/.git")
    find_package(Git REQUIRED)
    execute_process(
        COMMAND "${GIT_EXECUTABLE}" rev-parse --short HEAD
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
        RESULT_VARIABLE _git_ret
        OUTPUT_VARIABLE GIT_COMMIT_HASH
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    if(NOT _git_ret EQUAL 0)
        message(FATAL_ERROR
        "Failed to run git rev-parse in ${CMAKE_SOURCE_DIR}")
    endif()
    execute_process(
        COMMAND "${GIT_EXECUTABLE}" show --no-patch --format=%ct HEAD
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
        RESULT_VARIABLE _git_ret
        OUTPUT_VARIABLE GIT_COMMIT_TIME_UNIX_SEC
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    if(NOT _git_ret EQUAL 0)
        message(FATAL_ERROR
        "Failed to run git show --no-patch --format=%ct in ${CMAKE_SOURCE_DIR}")
    endif()
elseif(EXISTS "${CMAKE_SOURCE_DIR}/gitversion.cmake")
    # This file is generated by Makefile, before creating tarball.
    include("${CMAKE_SOURCE_DIR}/gitversion.cmake")
endif()

# ==============================================
# Project
# ==============================================
# Call project() here instead of top of file, because project command is not scriptable (when running `cmake -P`)

project("${PROJ_NAME}"
    VERSION ${PROJ_VERSION}
    DESCRIPTION ${PROJ_SUMMARY}
    HOMEPAGE_URL "https://github.com/kuanyui/qkiss"
    LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# https://gcc.gnu.org/onlinedocs//gcc/Static-Analyzer-Options.html
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_compile_options(-Wall -Wextra -Wunused -Wunused-variable -Wanalyzer-null-dereference -Werror
        # -fanalyzer
        # -Wno-analyzer-use-of-uninitialized-value
    )
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    add_compile_options(/W4)
    # NOMINMAX, WIN32_LEAN_AND_MEAN : disable some conflicted macros defined by Windows.h
    # _CRT_SECURE_NO_WARNINGS : cross-platform C++ project should use portable function instead of "safe version" defined only by Microsoft
    add_compile_definitions(
        NOMINMAX
        WIN32_LEAN_AND_MEAN
        _CRT_SECURE_NO_WARNINGS
    )
endif()


# ==============================================
# Option :: C++ Profiling
# ==============================================

option(ENABLE_PROFILING "Enable profiling (gprof / perf / hotsopt), should be used with `-DCMAKE_BUILD_TYPE=RelWithDebInfo`" OFF)

# IMPORTANT: these `CMAKE_CXX_*` must be used after project( ...LANGUAGES CXX)
if(ENABLE_PROFILING)
    # Keep frame pointers when possible (useful for profilers)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        add_compile_options(-fno-omit-frame-pointer)
        add_link_options(-fno-omit-frame-pointer)
    endif()

    if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
        # Linux: gprof / perf style profiling
        if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
            add_compile_options(-pg)
            add_link_options(-pg)
        endif()
    elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
        # macOS: Instruments / Time Profiler (no -pg)
        message(STATUS "Profiling enabled on macOS: using RelWithDebInfo + Instruments" )
    elseif(MSVC)
        # Windows: Visual Studio Profiler / ETW
        message(STATUS "Profiling enabled on Windows: use Visual Studio Profiler or ETW" )
    endif()
endif()


# ===============================================
# Prepare variables to store extra .h / .cpp / dynamic lib paths
# ===============================================
# Actually declaring variable is unnecessary in CMake but explicity is better then implicity.
# Be careful don't move this after `list(APPREND)` or its accumulated content will be clear.

set(EXTRA_DEFS)   # `target_compile_definitions()`, the defs passed to compiler, so that you can use `#ifdef XXX` in C++ code
set(EXTRA_INCS)   # `target_include_directories()`
set(EXTRA_LIBS)   # `target_link_libraries()`

# ==============================================
# Expand CMake macro (configure)
# ==============================================
configure_file("${CMAKE_SOURCE_DIR}/src/version.h.in"
"${CMAKE_BINARY_DIR}/version.h" @ONLY)
list(APPEND EXTRA_INCS "${CMAKE_BINARY_DIR}")

# ===============================================
# Package :: vcpkg compatibility (mainly for Windows)
# ===============================================
# If `CMAKE_TOOLCHAIN_FILE` is not defined by user, and system has `vcpkg`
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE AND DEFINED ENV{VCPKG_INSTALLATION_ROOT})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_INSTALLATION_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
endif()

if(WIN32)
    set(X_VCPKG_APPLOCAL_DEPS ON)  # auto copy DLL files to same folder of *.exe
endif()

# ===============================================
# Package :: Qt6
# ===============================================
find_package(QT NAMES Qt6 REQUIRED COMPONENTS Core Widgets LinguistTools Concurrent Multimedia)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Widgets LinguistTools Concurrent Multimedia)

# ===============================================
# Package :: iconv
# ===============================================
# find_library(ICONV_LIB NAMES iconv libiconv)    // Unreliable because may be built-in in glibc
# find_path(ICONV_INCLUDE_DIR NAMES iconv.h)
find_package(Iconv)
if(Iconv_FOUND)
  list(APPEND EXTRA_DEFS HAVE_ICONV)
  list(APPEND EXTRA_INCS ${Iconv_INCLUDE_DIRS})
  list(APPEND EXTRA_LIBS ${Iconv_LIBRARIES})
else()
    # Is this truly necessary?
    include(CheckFunctionExists)
    include(CheckIncludeFile)
    check_function_exists(iconv_open ICONV_EXISTS)
    check_include_file_cxx(iconv.h ICONV_H_EXISTS)
    if(ICONV_EXISTS AND ICONV_H_EXISTS)
        message(STATUS "Found iconv: ${ICONV_LIB}")
        list(APPEND EXTRA_DEFS HAVE_ICONV)
        list(APPEND EXTRA_INCS ${ICONV_INCLUDE_DIR})
        list(APPEND EXTRA_LIBS ${ICONV_LIB})
    else()
        message(STATUS "iconv not found.")
    endif()
endif()
# ===============================================
# Package :: ICU
# ===============================================
find_package(ICU COMPONENTS uc i18n data)
if(ICU_FOUND)
    list(APPEND EXTRA_DEFS HAVE_ICU)
    list(APPEND EXTRA_INCS ${ICU_INCLUDE_DIRS})
    list(APPEND EXTRA_LIBS ${ICU_LIBRARIES})
endif()

# ===============================================
# Package :: (iconv & ICU checking)
# ===============================================
if(NOT Iconv_FOUND AND NOT ICONV_EXISTS AND NOT ICU_FOUND)
    message(FATAL_ERROR "Unable to find `ICU` nor `Iconv`, aborted.")
endif()

# ===============================================
# Package :: LibArchive
# ===============================================
find_package(LibArchive)
if(LibArchive_FOUND)
    list(APPEND EXTRA_DEFS HAVE_LIBARCHIVE)
    list(APPEND EXTRA_INCS ${LibArchive_INCLUDE_DIRS})
    list(APPEND EXTRA_LIBS LibArchive::LibArchive)
endif()

# ===============================================
# Packaging related files
# ===============================================

configure_file("packaging/${PROJECT_NAME}.desktop.in" "${PROJ_GENERATED_DIR}/${PROJECT_NAME}.desktop" @ONLY)
configure_file("packaging/${PROJECT_NAME}.spec.in"    "${PROJ_GENERATED_DIR}/${PROJECT_NAME}.spec" @ONLY)
configure_file("packaging/PKGBUILD.in"                "${PROJ_GENERATED_DIR}/PKGBUILD" @ONLY)

file(GLOB debian_input_files
    RELATIVE "${CMAKE_SOURCE_DIR}/packaging/debian"
             "${CMAKE_SOURCE_DIR}/packaging/debian/*")

foreach(file_name IN LISTS debian_input_files)
    message(STATUS "${file_name}")
    set(src "packaging/debian/${file_name}")
    if(IS_DIRECTORY "${src}")
        continue()
    endif()
    if(file_name MATCHES "\\.in$")
        # Remove ".in"
        string(REGEX REPLACE "\\.in$" "" out_name "${file_name}")
        set(dst "${PROJ_GENERATED_DIR}/debian/${out_name}")
        configure_file("${src}" "${dst}" @ONLY)
    else()
        set(dst "${PROJ_GENERATED_DIR}/debian/${file_name}")
        file(COPY "${src}" DESTINATION "${PROJ_GENERATED_DIR}/debian" )
    endif()
endforeach()
file(COPY   "${CMAKE_SOURCE_DIR}/packaging/debian/source/format"
     DESTINATION "${PROJ_GENERATED_DIR}/debian/source/")

# NSIS files, use CRLF
configure_file("packaging/windows/install.nsh.in"   "${PROJ_GENERATED_DIR}/windows/install.nsh"   @ONLY NEWLINE_STYLE CRLF)
configure_file("packaging/windows/uninstall.nsh.in" "${PROJ_GENERATED_DIR}/windows/uninstall.nsh" @ONLY NEWLINE_STYLE CRLF)

# ===============================================
# main
# ===============================================

set(PROJECT_SOURCES
        src/main.cpp
        src/mainwindow.cpp
        src/mainwindow.h
        src/mainwindow.ui
        src/archivefile.h
        src/archivefile.cpp
)


qt_add_executable("${PROJ_NAME}"
    MANUAL_FINALIZATION
    ${PROJECT_SOURCES}
    ${WIN_ICO_RESOURCE}
    resources/application.qrc
    ${ICON_PNG_GENERATED_QRC}
    src/parsedcnf.h src/parsedcnf.cpp
    src/enums.h src/enums.cpp
    src/encoding.h src/encoding.cpp
    src/parsedkcf.h src/parsedkcf.cpp
    src/parsedcel.h src/parsedcel.cpp
    src/gkissview.h src/gkissview.cpp
    src/gkissitem.h src/gkissitem.cpp
    src/gkissscene.h src/gkissscene.cpp
    src/configfile.h src/configfile.cpp
    src/fkiss.h src/fkiss.cpp
    src/fkissvariant.h src/fkissvariant.cpp

    src/fkissparser.h src/fkissparser.cpp
    src/fkissruntime.h src/fkissruntime.cpp
    src/archiveprocessorexternallha.h src/archiveprocessorexternallha.cpp
    src/filesystembrowser.h src/filesystembrowser.cpp
    src/logviewer.h src/logviewer.cpp
    src/innerfilesbrowser.h src/innerfilesbrowser.cpp
    src/codetextedit.h src/codetextedit.cpp
    src/codetexteditlinenumberarea.h src/codetexteditlinenumberarea.cpp
    src/codetexteditsyntaxhighlighter.h src/codetexteditsyntaxhighlighter.cpp
    src/thememanager.h src/thememanager.cpp
    src/fileiconprovider.h src/fileiconprovider.cpp
    src/iconbuttondelegate.h src/iconbuttondelegate.cpp
    src/abstractarchiveprocessor.h src/abstractarchiveprocessor.cpp
    src/archiveprocessorlibarchive.h src/archiveprocessorlibarchive.cpp
    src/archiveprocessorfactory.h src/archiveprocessorfactory.cpp
    src/audioplayer.h src/audioplayer.cpp
    src/iteminspector.h src/iteminspector.cpp
    src/texteditsearchbar.h src/texteditsearchbar.cpp
    src/codeeditor.h src/codeeditor.cpp
    src/archiveprocessorexternalunar.h src/archiveprocessorexternalunar.cpp
    src/archiveprocessorcnfinfilesystem.h src/archiveprocessorcnfinfilesystem.cpp
    src/preferencedialog.h src/preferencedialog.cpp
    src/zcombobox.h src/zcombobox.cpp
    src/zgroupbox.h src/zgroupbox.cpp
    src/aboutdialog.h src/aboutdialog.cpp
    src/zlabel.h src/zlabel.cpp
    src/translationmanager.h src/translationmanager.cpp
    src/casepreservedstring.h src/casepreservedstring.cpp
    src/ztranslator.h src/ztranslator.cpp
    src/ztreewidget.h src/ztreewidget.cpp

)
add_dependencies("${PROJ_NAME}" icontheme)
if (WIN32)
    add_dependencies("${PROJ_NAME}" generate_windows_icons)
endif()
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET qkiss APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation


# ===============================================
# i18n
# ===============================================
set(ALL_TR_LANGS ja_JP ja_KS ja_OJ ja_AR ja_MK ja_ME zh_TW)

# if(QT_VERSION VERSION_GREATER_EQUAL "6.7")
# https://doc.qt.io/Qt-6/qtlinguist-cmake-qt-add-translations.html#sources-and-include-directories
# Since Qt6.3
# qt_standard_project_setup(
#     I18N_SOURCE_LANGUAGE en_US   # This is buggy
#     I18N_TRANSLATED_LANGUAGES ${ALL_TR_LANGS}
#     )

# https://doc.qt.io/qt-6/qtlinguist-cmake-qt-add-translations.html
# Since Qt 6.2
#
# 1. This function will create the CMake target `update_translations` that scans
#    all source files with `lupdate` and creates and updates the .ts files.
# 2. This function will create the CMake target `release_translations` that
#    generates the .qm files from the .ts files. This target is built by
#    default.
# qt_add_translations(${PROJECT_NAME}      # add translations to `${PROJECT_NAME}` target
#     # `lupdate` will scan all `tr()` and output .ts file to:
#     TS_OUTPUT_DIRECTORY i18n             # (since Qt6.9) i18n/
#     TS_FILE_BASE ${PROJECT_NAME}         # (since Qt6.7) ${TS_FILE_BASE}_${LANG}.ts
#     LUPDATE_OPTIONS                      # (since Qt6.2, deprecated)
#         -no-obsolete
#     IMMEDIATE_CALL                       # (since Qt6.7)
# )
# By default, the generated .qm files are embedded in a Qt resource that will be
# linked into the targets passed with TARGETS. The files in the resource are
# accessible under the resource prefix "/i18n".


# Qt6.2 ~ Qt6.7
set(TS_FILES "")
foreach(LANG   IN LISTS   ALL_TR_LANGS)
    list(APPEND TS_FILES "i18n/${PROJECT_NAME}_${LANG}.ts")
endforeach()
# Scan all source files with `lupdate` to generate *.ts (Create a CMake umbrella target `update_translations`)
qt_add_lupdate(${PROJECT_NAME} TS_FILES ${TS_FILES}
    # OPTIONS -no-obsolete   # remove obsolete strings from all *.ts
)
# Compile *.ts -> *.qm (Create a CMake umbrella target `release_translations`)
qt_add_lrelease(${PROJECT_NAME} TS_FILES ${TS_FILES}
    QM_FILES_OUTPUT_VARIABLE QM_FILES
)
# Add &.qm files to target
target_sources(${PROJECT_NAME} PRIVATE ${QM_FILES})

# Qt resource doesn't accept absolute path as resource path,
# so guild-generated files (.qm) requires QT_RESOURCE_ALIAS
foreach(qm IN LISTS QM_FILES)
    get_filename_component(qm_name "${qm}" NAME)
    set_source_files_properties("${qm}"
        PROPERTIES
            QT_RESOURCE_ALIAS "${qm_name}"
    )
endforeach()
# *.qm -> *.rcc
qt_add_resources(${PROJECT_NAME} translations
    PREFIX "/i18n"
    FILES ${QM_FILES}
)

# ===============================================
# (i18n ends here)
# ===============================================

target_compile_definitions(qkiss PRIVATE ${EXTRA_DEFS})
target_include_directories(qkiss PRIVATE ${EXTRA_INCS})

target_link_libraries(qkiss PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Concurrent
    Qt${QT_VERSION_MAJOR}::Multimedia
    ${EXTRA_LIBS}
    )


# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.qkiss)
endif()
set_target_properties(qkiss PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

if(QT_VERSION_MAJOR EQUAL 6)
    # Before install(), after target_link_libraries()
    qt_finalize_executable(qkiss)  # Must before include(CPack)
endif()


# ===============================================
# Install
# ===============================================

include(GNUInstallDirs)  # This is actually cross-platform
# **NOT** all `CMAKE_INSTALL_*` are provided during `cmake --install` time.
#
# - CMAKE_INSTALL_PREFIX      abs path. Visible during install time.
# - CMAKE_INSTALL_BINDIR      rel path, it should be `bin/`. **BUT this is invisible during install time**
# - CMAKE_INSTALL_LIBDIR      rel path, it should be `lib/`. **BUT this is invisible during install time**
# - CMAKE_INSTALL_INCLUDEDIR  rel path, it should be `include/`. **BUT this is invisible during install time**

if(WIN32)
    # Force overwrite CMAKE_INSTALL_BINDIR
    # On Windows, *.exe and *.dll are usually placed under app's folder root, instead of bin/
    set(CMAKE_INSTALL_BINDIR "." CACHE STRING "Install dir (relpath) for executables" FORCE)
endif()

install(TARGETS ${PROJECT_NAME}
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
# CPack will pack `Runtime`, `Devel`, `Docs` as different `COMPONENT`, `install()` them separately.

# file(COPY samples DESTINATION ${CMAKE_BINARY_DIR})



# These `install(CODE ...)` and `install(SCRIPT ...)` actually is not installing files via CMake `install()`. Instead:
#  - `install(CODE ...)` means "run this code only in installing time instead of configure & build time",
#    that is to say, copy the string without any reinterpretion into `cmake_install.cmake`.
#  - `install(SCRIPT <cmake_file_path>)`
#
# -------------------------------------------

if(UNIX AND NOT APPLE AND NOT WIN32)
    # Linux / BSD

    # Copy built qkiss main app icons into an independent dir for installation.
    # NOTE: `install()` will only be triggered when `cmake --install` or `make install` or (rpm spec's) `%cmake_install`, so you won't see them when developing.

    # IMPORTANT: the paths inside `install()` are determined only when `cmake`
    # configure time. So, if icons are not generated when first-time `cmake`
    # (that is to say, when the result of `file(GLOB_RECURSE result "*.png")` is
    # empty), the icons may be potentially unable to be installed via `cmake
    # --install`.


    # XDG icon theme :: png files
    message(STATUS "[DEBUG] CMAKE_SOURCE_DIR === ${CMAKE_SOURCE_DIR}")
    message(STATUS "[DEBUG] ICON_PNG_XDG_THEME_DIR === ${ICON_PNG_XDG_THEME_DIR}")
    file(GLOB_RECURSE iconPngFiles "${ICON_PNG_XDG_THEME_DIR}/*/apps/qkiss.png")
    message(STATUS "[DEBUG] Found PNG icons to install = ${iconPngFiles}")
    foreach(iconAbsFilePath ${iconPngFiles})
        get_filename_component(iconAbsDirPath ${iconAbsFilePath} DIRECTORY)
        string(REPLACE "${ICON_PNG_XDG_THEME_DIR}/" "" iconRelFilePath ${iconAbsDirPath})
        # message(STATUS "[DEBUG] iconAbsDirPath = ${iconAbsDirPath}")
        # message(STATUS "[DEBUG] iconAbsFilePath = ${iconAbsFilePath}")
        # message(STATUS "[DEBUG] iconRelFilePath = ${iconRelFilePath}")
        install(FILES ${iconAbsFilePath} DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/${iconRelFilePath})
    endforeach()

    # XDG icon theme :: svg files
    file(GLOB_RECURSE iconSvgFiles "${ICON_SVG_XDG_THEME_DIR}/*/apps/qkiss.svg")
    message(STATUS "[DEBUG] Found SVG icons to install = ${iconSvgFiles}")
    foreach(iconAbsFilePath ${iconSvgFiles})
        get_filename_component(iconAbsDirPath ${iconAbsFilePath} DIRECTORY)
        string(REPLACE "${ICON_SVG_XDG_THEME_DIR}/" "" iconRelFilePath ${iconAbsDirPath})
        install(FILES ${iconAbsFilePath} DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/${iconRelFilePath})
    endforeach()

    # XDG desktop file
    install(FILES "${PROJ_GENERATED_DIR}/${PROJECT_NAME}.desktop" DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)

elseif(WIN32)
    # ===================================================
    # `CPACK_*` variables
    # ===================================================

    # Default value of ${CPACK_PACKAGE_FILE_NAME} is
    # ${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CPACK_SYSTEM_NAME}
    set(CPACK_PACKAGE_NAME "${PROJ_DISPLAY_NAME}")
    set(CPACK_PACKAGE_VENDOR "QKiSS Project")  # only used in "Vendor" field in Installer.
    set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})  # to display
    set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})  # to auto compare version
    set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
    set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})
    set(CPACK_PACKAGE_HOMEPAGE_URL "https://github.com/kuanyui/qkiss")
    set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${PROJ_SUMMARY}")
    set(CPACK_PACKAGE_DESCRIPTION "${PROJ_DESCRIPTION}")

    set(CPACK_SOURCE_PACKAGE_FILE_NAME "${PROJECT_NAME}-${PROJECT_VERSION}")  # source code tarball file name
    set(CPACK_PACKAGE_FILE_NAME "${PROJECT_NAME}-${PROJECT_VERSION}-${CPACK_SYSTEM_NAME}")  # Installer / Portable ZIP file name
    set(CPACK_NSIS_DISPLAY_NAME "${PROJ_DISPLAY_NAME} ${PROJECT_VERSION} Installer")   # Window title of installer

    set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")
    set(CPACK_GENERATOR "NSIS;ZIP")
    # ===================================================
    # `install()` external .exe (lha, unar)
    # ===================================================

    set(WIN_EXTERNAL_BIN_DEPS_DIR "" CACHE PATH "[Windows only] Path to the directory containing external binary dependencies (lha, unar)")

    # Search external dependencies in variable `WIN_EXTERNAL_BIN_DEPS_DIR`, falling back to System PATH if not found.
    find_program(LHA_EXECUTABLE lha HINTS "${WIN_EXTERNAL_BIN_DEPS_DIR}")
    find_program(UNAR_EXECUTABLE unar HINTS "${WIN_EXTERNAL_BIN_DEPS_DIR}")
    find_file(FOUNDATION_DLL "Foundation.1.0.dll" HINTS "${WIN_EXTERNAL_BIN_DEPS_DIR}")

    if(LHA_EXECUTABLE AND UNAR_EXECUTABLE AND FOUNDATION_DLL)
        install(FILES "${LHA_EXECUTABLE}" "${UNAR_EXECUTABLE}" "${FOUNDATION_DLL}" DESTINATION ${CMAKE_INSTALL_BINDIR})
        message(STATUS "External dependencies (lha/unar) found and scheduled for installation.")
    else()
        message(FATAL_ERROR "External dependencies (lha/unar) not found in WIN_EXTERNAL_BIN_DEPS_DIR nor PATH! Installer will be incomplete.")
    endif()

    # ===================================================
    # `install()` Qt runtime DLLs via `windeployqt`
    # ===================================================



    # Get `windeployqt` to pick DLLs (according to the above `find_package(Qt6 REQUIRED COMPONENTS Core ...)`)
    # To reliably get the correct version of `windeployqt` installed on this computer, need to get the folder of `qmake` first,
    # which has been found by `find_package(Qt6 ...)`
    get_target_property(_qmake_executable Qt6::qmake IMPORTED_LOCATION)
    get_filename_component(_qt_bin_dir "${_qmake_executable}" DIRECTORY)
    find_program(WINDEPLOYQT_EXECUTABLE_CONFIGURED windeployqt HINTS "${_qt_bin_dir}" REQUIRED)

    configure_file(
          "${CMAKE_SOURCE_DIR}/packaging/windows/deploy_qt_runtime.cmake.in"
        "${PROJ_GENERATED_DIR}/packaging/windows/deploy_qt_runtime.cmake"
        @ONLY)
    install(SCRIPT "${PROJ_GENERATED_DIR}/packaging/windows/deploy_qt_runtime.cmake")


    # - Qt > 6.3 provides `qt_generate_deploy_app_script()` means to replace `windeployqt` to
    #   generate a script to `install()` all DLLs. NEVERTHELESS!!! this shit is a blackbox which
    #   actually always install binaries into `bin/`. fucking idiot over-engineered Qt tools.
    #   So I decide not to use this shit here.
    # - This `qt_generate_deploy_app_script()` doesn't supports "Visual Studio 17 2022" as generator
    #   (because it always generates Multi-Config), use Ninja (Single-Config) instead.

    # qt_generate_deploy_app_script(
    #     TARGET qkiss
    #     OUTPUT_SCRIPT deploy_script.cmake
    #     NO_TRANSLATIONS
    #     # --compiler-runtime will be automatically handled
    # )
    # install(SCRIPT ${deploy_script.cmake})

    # ===================================================
    # `install()` the third-party runtime DLLs in vcpkg
    # ===================================================

    # `bumpbin.exe` is provided by MSVC. These `find_program()` must be run during configure time,
    find_program(DUMPBIN_EXECUTABLE_CONFIGURED dumpbin REQUIRED)
    find_program(DEPENDENCIES_EXECUTABLE_CONFIGURED Dependencies REQUIRED)
    # requires -DCMAKE_TOOLCHAIN_FILE="$env:VCPKG_INSTALLATION_ROOT/scripts/buildsystems/vcpkg.cmake"
    configure_file(
        "${CMAKE_SOURCE_DIR}/packaging/windows/deploy_vcpkg_runtime__dumpbin.cmake.in"
      "${PROJ_GENERATED_DIR}/packaging/windows/deploy_vcpkg_runtime.cmake"
    @ONLY)
    install(SCRIPT "${PROJ_GENERATED_DIR}/packaging/windows/deploy_vcpkg_runtime.cmake")
    # Try to rely on `X_VCPKG_APPLOCAL_DEPS` + `find_package()`
    install(FILES $<TARGET_RUNTIME_DLLS:${CMAKE_PROJECT_NAME}> DESTINATION "${CMAKE_INSTALL_BINDIR}")
    # ===================================================
    # Fuck Windows, fuck NSIS, fuck CMake.
    # ===================================================

    # File association to `*.lzh` (for NSIS only)
    set(CPACK_NSIS_FILE_ASSOCIATIONS "lzh")

    # NSIS parser may be incompatible with `/` path, use Windows native `\` should be safer... Maybe...
    file(TO_NATIVE_PATH "${PROJ_GENERATED_DIR}/windows/install.nsh" _WIN_NATIVE_PATH_NSIS_INSTALL)
    file(TO_NATIVE_PATH "${PROJ_GENERATED_DIR}/windows/uninstall.nsh" _WIN_NATIVE_PATH_NSIS_UNINSTALL)
    # Ridiculous "NATIVE". Fuck Windows again. FUUUUCK.
    string(REPLACE "\\" "\\\\" _WIN_NATIVE_PATH_NSIS_INSTALL "${_WIN_NATIVE_PATH_NSIS_INSTALL}")
    string(REPLACE "\\" "\\\\" _WIN_NATIVE_PATH_NSIS_UNINSTALL "${_WIN_NATIVE_PATH_NSIS_UNINSTALL}")

    # Fuck CMake, WHAT THE FUCK IS THIS STUPID BEHAVIOR???
    unset(CPACK_NSIS_EXTRA_INSTALL_COMMANDS)
    unset(CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS)

    list(APPEND CPACK_NSIS_EXTRA_INSTALL_COMMANDS "!include ${_WIN_NATIVE_PATH_NSIS_INSTALL}")
    list(APPEND CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS "!include ${_WIN_NATIVE_PATH_NSIS_UNINSTALL}")

    set(CPACK_NSIS_MUI_ICON ${GEN_WIN_ICO_FILE_INSTALLER})  # installer itself
    set(CPACK_NSIS_MUI_UNIICON ${GEN_WIN_ICO_FILE_UNINSTALLER})  # uninstaller itself
    set(CPACK_NSIS_INSTALLED_ICON_NAME "qkiss.exe")
    set(CPACK_NSIS_MENU_LINKS "qkiss.exe" "QKiSS") # Start Menu
    set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)  # Force uninstall old version before installing
    include(CPack)  # execute CPack to read the defined variables.
endif()

