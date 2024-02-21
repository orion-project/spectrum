#-------------------------------------------------
#
# Project created by QtCreator 2017-07-17T11:28:49
#
#-------------------------------------------------

QT += core gui widgets network

CONFIG += c++17
QMAKE_CXXFLAGS_WARN_ON += -Wno-unknown-pragmas

#------------------------------------------------------------
# Definition of output

TARGET = spectrum
TEMPLATE = app
DESTDIR = $$_PRO_FILE_PWD_/bin

#------------------------------------------------------------
# Submodules

#--------
# orion (https://github.com/orion-project/orion-qt)
ORION = $$_PRO_FILE_PWD_/libs/orion
include($$ORION/orion.pri)

#--------
# custom-plot-lab (https://github.com/orion-project/custom-plot-lab)

# qcustomplot.cpp is so large that it even fails to build in debug mode
# If debug mode is required, you have to use QCustomPlot as shared library:
# cd libs\custom-plot-lab\qcustomplot; qmake; mingw32-make release
# Then enable "qcustomplotlab_shared" options and rebuild rezonator
#win32: CONFIG += qcustomplotlab_shared

include($$_PRO_FILE_PWD_/libs/custom-plot-lab/custom-plot-lab.pri)

#------------------------------------------------------------
# Version information

include(release/version.pri)

#------------------------------------------------------------

win32: RC_FILE = src/app.rc

#------------------------------------------------------------
# Sources

RESOURCES += \
    src/app.qrc

SOURCES += src/main.cpp\
    src/CsvConfigDialog.cpp \
    src/CustomPrefs.cpp \
    src/app/AppSettings.cpp \
    src/app/HelpSystem.cpp \
    src/MainWindow.cpp \
    src/OpenFileDlg.cpp \
    src/PlotWindow.cpp \
    src/Operations.cpp \
    src/app/PersistentState.cpp \
    src/core/DataExporters.cpp \
    src/core/DataReaders.cpp \
    src/core/DataSources.cpp \
    src/core/Graph.cpp \
    src/core/GraphMath.cpp \
    src/core/Modifiers.cpp \
    src/widgets/DataGridPanel.cpp \
    src/windows/HelpWindow.cpp

HEADERS  += \
    src/CsvConfigDialog.h \
    src/CustomPrefs.h \
    src/app/AppSettings.h \
    src/app/HelpSystem.h \
    src/MainWindow.h \
    src/OpenFileDlg.h \
    src/PlotWindow.h \
    src/Operations.h \
    src/app/PersistentState.h \
    src/core/BaseTypes.h \
    src/core/DataExporters.h \
    src/core/DataReaders.h \
    src/core/DataSources.h \
    src/core/Graph.h \
    src/core/GraphMath.h \
    src/core/Modifiers.h \
    src/widgets/DataGridPanel.h \
    src/windows/HelpWindow.h
