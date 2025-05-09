cmake_minimum_required(VERSION 3.10)

project(qkiss VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# https://gcc.gnu.org/onlinedocs//gcc/Static-Analyzer-Options.html
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_compile_options(-Wall -Wextra -Wunused -Wunused-variable -Wanalyzer-null-dereference -Werror
        # -fanalyzer
        # -Wno-analyzer-use-of-uninitialized-value
    )
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    add_compile_options(/W4)
endif()


find_package(QT NAMES Qt6 REQUIRED COMPONENTS Core Widgets LinguistTools Concurrent Multimedia)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Widgets LinguistTools Concurrent Multimedia)

set(EXTRA_INCS)
set(EXTRA_DEFS)
set(EXTRA_LIBS)


# ===============================================
# iconv
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
# ICU
# ===============================================
find_package(ICU COMPONENTS uc i18n data)
if(ICU_FOUND)
    list(APPEND EXTRA_DEFS HAVE_ICU)
    list(APPEND EXTRA_INCS ${ICU_INCLUDE_DIRS})
    list(APPEND EXTRA_LIBS ${ICU_LIBRARIES})
endif()

# ===============================================
# LibArchive
# ===============================================
find_package(LibArchive)
if(LibArchive_FOUND)
    list(APPEND EXTRA_DEFS HAVE_LIBARCHIVE)
    list(APPEND EXTRA_INCS ${LibArchive_INCLUDE_DIRS})
    list(APPEND EXTRA_LIBS LibArchive::LibArchive)
endif()

# ===============================================
### Build Icons
# ===============================================
find_package(Python3 REQUIRED Interpreter)
set(RESOURCES_DIR "${CMAKE_SOURCE_DIR}/resources")
set(ICON_REPO_DIR "${CMAKE_SOURCE_DIR}/resources/tango-icon-remixer")
set(ICON_BUILD_SCRIPT "${ICON_REPO_DIR}/build_icons.py")
execute_process(
    COMMAND ${Python3_EXECUTABLE} ${ICON_BUILD_SCRIPT} --included-icon-file "${RESOURCES_DIR}/included_icons.txt" --extra-icon-theme "AppIcon:${RESOURCES_DIR}/appicon" --output-theme-name "CustomizedTango"
)
set_property(
    DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${RESOURCES_DIR}/included_icons.txt"
)

set(TS_FILES qkiss_en_US.ts)

set(PROJECT_SOURCES
        src/main.cpp
        src/mainwindow.cpp
        src/mainwindow.h
        src/mainwindow.ui
        src/archivefile.h
        src/archivefile.cpp
        i18n/${TS_FILES}
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(qkiss
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        resources/application.qrc
        resources/tango-icon-remixer/dist/04_FINAL_PNG_THEME/CustomizedTango/index.qrc   # REFACTOR: path
        src/parsedcnf.h src/parsedcnf.cpp
        src/enums.h
        src/encoding.h src/encoding.cpp
        src/parsedkcf.h src/parsedkcf.cpp
        src/parsedcel.h src/parsedcel.cpp
        src/gkissview.h src/gkissview.cpp
        src/gkissitem.h src/gkissitem.cpp
        src/gkissscene.h src/gkissscene.cpp
        src/globalstate.h src/globalstate.cpp
        src/fkiss.h src/fkiss.cpp
        src/fkissvariant.h src/fkissvariant.cpp

        src/fkissparser.h src/fkissparser.cpp
        src/fkissruntime.h src/fkissruntime.cpp
        src/archiveprocessorexternallha.h src/archiveprocessorexternallha.cpp
        src/filesystembrowser.h src/filesystembrowser.cpp
        src/logviewer.h src/logviewer.cpp
        src/innerfilesbrowser.h src/innerfilesbrowser.cpp
        src/codeeditor.h src/codeeditor.cpp
        src/codeeditorlinenumberarea.h src/codeeditorlinenumberarea.cpp
        src/codeeditorsyntaxhighlighter.h src/codeeditorsyntaxhighlighter.cpp
        src/thememanager.h src/thememanager.cpp
        src/fileiconprovider.h src/fileiconprovider.cpp
        src/iconbuttondelegate.h src/iconbuttondelegate.cpp
        src/abstractarchiveprocessor.h src/abstractarchiveprocessor.cpp
        src/archiveprocessorlibarchive.h src/archiveprocessorlibarchive.cpp
        src/archiveprocessorfactory.h src/archiveprocessorfactory.cpp
        src/audioplayer.h src/audioplayer.cpp
        src/iteminspector.h src/iteminspector.cpp

    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET qkiss APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation

    qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
else()
    if(ANDROID)
        add_library(qkiss SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(qkiss
            ${PROJECT_SOURCES}
        )
    endif()

    qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
endif()

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

include(GNUInstallDirs)
install(TARGETS qkiss
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
file(COPY
    samples DESTINATION ${CMAKE_BINARY_DIR})

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(qkiss)
endif()
