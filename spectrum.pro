#-------------------------------------------------
#
# Project created by QtCreator 2017-07-17T11:28:49
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = spectrum
TEMPLATE = app
DESTDIR = $$_PRO_FILE_PWD_/bin

win32: RC_FILE = src/app.rc

include(orion/orion.pri)
include(custom-plot-lab/custom-plot-lab.pri)
include(release/version.pri)

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

RESOURCES += src/images.qrc

SOURCES += src/main.cpp\
    src/CustomPrefs.cpp \
    src/HelpSystem.cpp \
        src/MainWindow.cpp \
    src/PlotWindow.cpp \
    src/Operations.cpp \
    src/core/DataSources.cpp \
    src/core/Graph.cpp \
    src/core/GraphMath.cpp \
    src/core/Modificators.cpp \
    src/widgets/DataGridPanel.cpp \
    src/Settings.cpp

HEADERS  += \
    src/CustomPrefs.h \
    src/HelpSystem.h \
        src/MainWindow.h \
    src/PlotWindow.h \
    src/Operations.h \
    src/core/BaseTypes.h \
    src/core/DataSources.h \
    src/core/Graph.h \
    src/core/GraphMath.h \
    src/core/Modificators.h \
    src/widgets/DataGridPanel.h \
    src/Settings.h
