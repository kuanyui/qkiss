cmake_minimum_required(VERSION 3.5)

project(qtkiss VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_compile_options(-Wall -Wextra -Wunused -Wunused-variable)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    add_compile_options(/W4)
endif()


find_package(QT NAMES Qt6 REQUIRED COMPONENTS Core Widgets LinguistTools)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Widgets LinguistTools)
find_package(ICU REQUIRED COMPONENTS uc i18n data)

if(ICU_FOUND)
    include_directories(${ICU_INCLUDE_DIRS})
endif()

set(TS_FILES qtkiss_en_US.ts)

set(PROJECT_SOURCES
        src/main.cpp
        src/mainwindow.cpp
        src/mainwindow.h
        src/mainwindow.ui
        src/archivedfile.h
        src/archivedfile.cpp
        i18n/${TS_FILES}
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(qtkiss
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        resources/application.qrc
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
        src/archiveprocessor.h src/archiveprocessor.cpp
        src/filesystembrowser.h src/filesystembrowser.cpp

    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET qtkiss APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation

    qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
else()
    if(ANDROID)
        add_library(qtkiss SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(qtkiss
            ${PROJECT_SOURCES}
        )
    endif()

    qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
endif()


target_link_libraries(qtkiss PRIVATE Qt${QT_VERSION_MAJOR}::Widgets ${ICU_LIBRARIES})

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.qtkiss)
endif()
set_target_properties(qtkiss PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS qtkiss
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
file(COPY
    samples DESTINATION ${CMAKE_BINARY_DIR})

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(qtkiss)
endif()
