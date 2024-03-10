cmake_minimum_required(VERSION 3.5)

file(READ "release/version.txt" APP_VER_FULL)

project(spectrum VERSION ${APP_VER_FULL} LANGUAGES CXX)

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
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets PrintSupport Network)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets PrintSupport Network)

set(PROJECT_SOURCES
    ${CMAKE_CURRENT_BINARY_DIR}/version.rc
    src/app.rc
    src/app.qrc
    src/CustomPrefs.cpp
    src/main.cpp
    src/Operations.cpp
    src/app/AppSettings.cpp
    src/app/HelpSystem.cpp
    src/app/PersistentState.cpp
    src/core/BaseTypes.cpp
    src/core/DataExporters.cpp
    src/core/DataReaders.cpp
    src/core/DataSources.cpp
    src/core/Graph.cpp
    src/core/GraphMath.cpp
    src/core/Modifiers.cpp
    src/dialogs/CsvConfigDialog.cpp
    src/dialogs/OpenFileDlg.cpp
    src/widgets/DataGridPanel.cpp
    src/widgets/RangeEditor.cpp
    src/windows/HelpWindow.cpp
    src/windows/MainWindow.cpp
    src/windows/PlotWindow.cpp
)

add_subdirectory(libs/orion)
add_subdirectory(libs/custom-plot-lab)

set(LIB_RESOURCES
    libs/orion/resources.qrc
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

target_link_libraries(${PROJECT_NAME} PRIVATE
    orion
    custom-plot-lab
    Qt::Widgets
    Qt::Network
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
    APP_VER_CODENAME="dev"
    QT_USE_QSTRINGBUILDER
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(${PROJECT_NAME})
endif()
