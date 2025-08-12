cmake_minimum_required(VERSION 3.5)

# set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)

# Remove per-configuration suffix from output directory added by VSCode
# https://stackoverflow.com/questions/47175912/using-cmake-how-to-stop-the-debug-and-release-subdirectories
# https://cmake.org/cmake/help/v3.4/prop_tgt/RUNTIME_OUTPUT_DIRECTORY.html#prop_tgt:RUNTIME_OUTPUT_DIRECTORY
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY $<1:${CMAKE_CURRENT_SOURCE_DIR}/bin>)

file(READ "release/version.txt" APP_VER_FULL)

project(spectrum VERSION ${APP_VER_FULL} LANGUAGES CXX)

# Set policies for modern CMake behavior
# CMP0077: option() honors normal variables - allows parent projects to override options
# CMP0074: find_package() uses <PackageName>_ROOT variables for finding packages
if(POLICY CMP0077)
    cmake_policy(SET CMP0077 NEW)
endif()
if(POLICY CMP0074)
    cmake_policy(SET CMP0074 NEW)
endif() 

string(TIMESTAMP BUILD_DATE "%Y-%m-%d")
string(TIMESTAMP APP_VER_YEAR "%Y")

string(REPLACE "." ";" APP_VER_LIST ${APP_VER_FULL})
list(GET APP_VER_LIST 0 APP_VER_MAJOR)
list(GET APP_VER_LIST 1 APP_VER_MINOR)
list(GET APP_VER_LIST 2 APP_VER_PATCH)

configure_file(
    release/version.rc.in
    ${CMAKE_CURRENT_BINARY_DIR}/version.rc
    @ONLY
)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Disable MSVC warnings about non-standard STL extensions globally
if(MSVC)
    add_compile_definitions(_SILENCE_STDEXT_ARR_ITERS_DEPRECATION_WARNING)
    add_compile_options(/wd4043)  # Disable STL4043 warning about stdext::checked_array_iterator
endif()  

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets PrintSupport Network)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets PrintSupport Network)

set(PROJECT_SOURCES
    ${CMAKE_CURRENT_BINARY_DIR}/version.rc
    src/app.rc
    src/app.qrc
    src/CustomPrefs.h src/CustomPrefs.cpp
    src/main.cpp
    src/Operations.h src/Operations.cpp
    src/app/AppSettings.h src/app/AppSettings.cpp
    src/app/HelpSystem.h src/app/HelpSystem.cpp
    src/app/PersistentState.h src/app/PersistentState.cpp
    src/core/BaseTypes.h src/core/BaseTypes.cpp
    src/core/DataExporters.h src/core/DataExporters.cpp
    src/core/DataReaders.h src/core/DataReaders.cpp
    src/core/DataSources.h src/core/DataSources.cpp
    src/core/FileUtils.h src/core/FileUtils.cpp
    src/core/GraphMath.h src/core/GraphMath.cpp
    src/core/Modifiers.h src/core/Modifiers.cpp
    src/core/Project.h src/core/Project.cpp
    src/core/ProjectFile.h src/core/ProjectFile.cpp
    src/dialogs/CsvConfigDialog.h src/dialogs/CsvConfigDialog.cpp
    src/dialogs/OpenFileDlg.h src/dialogs/OpenFileDlg.cpp
    src/tests/test_DataReaders.cpp
    src/tests/test_GraphMath.cpp
    src/tests/TestSuite.h
    src/widgets/DataGridPanel.h src/widgets/DataGridPanel.cpp
    src/widgets/RangeEditor.h src/widgets/RangeEditor.cpp
    src/windows/MainWindow.h src/windows/MainWindow.cpp
    src/windows/PlotWindow.h src/windows/PlotWindow.cpp
)

set(ORI_WITH_TESTS ON)
set(ORI_WITH_ZIP_HELP ON)
add_subdirectory(libs/orion)
add_subdirectory(libs/custom-plot-lab)

set(LIB_RESOURCES
    libs/orion/resources.qrc
    libs/orion/testing/test.qrc
    libs/custom-plot-lab/qcpl_images.qrc
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(${PROJECT_NAME}
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        ${LIB_RESOURCES}
    )
else()
    add_executable(${PROJECT_NAME}
        ${PROJECT_SOURCES}
        ${LIB_RESOURCES}
    )
endif()

find_package(libzip CONFIG REQUIRED)

target_link_libraries(${PROJECT_NAME} PRIVATE
    orion
    custom-plot-lab
    Qt::Widgets
    Qt::Network
    libzip::zip
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER org.orion-project.spectrum
    MACOSX_BUNDLE_BUNDLE_VERSION ${APP_VER_FULL}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${APP_VERSION_MAJOR}.${APP_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
    BUILD_DATE="${BUILD_DATE}"
    APP_VER_MAJOR=${APP_VER_MAJOR}
    APP_VER_MINOR=${APP_VER_MINOR}
    APP_VER_PATCH=${APP_VER_PATCH}
    APP_VER_YEAR=${APP_VER_YEAR}
    QT_USE_QSTRINGBUILDER
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(${PROJECT_NAME})
endif()
