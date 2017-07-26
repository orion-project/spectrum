#-------------------------------------------------
#
# Project created by QtCreator 2017-07-17T11:28:49
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = spectrum
TEMPLATE = app
DESTDIR = $$_PRO_FILE_PWD_/bin

include("orion/orion.pri")
include("custom-plot-lab/custom-plot-lab.pri")

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

RESOURCES += images.qrc

SOURCES += src/main.cpp\
        src/MainWindow.cpp \
    src/PlotWindow.cpp \
    src/Operations.cpp \
    src/core/Graph.cpp \
    src/widgets/DataGridPanel.cpp \
    src/Settings.cpp \
    src/funcs/FuncRandomSample.cpp \
    src/funcs/FuncPlotText.cpp \
    src/funcs/FuncPlotTextFile.cpp \
    src/funcs/FuncPlotClipboard.cpp

HEADERS  += \
        src/MainWindow.h \
    src/PlotWindow.h \
    src/Operations.h \
    src/core/Graph.h \
    src/widgets/DataGridPanel.h \
    src/Settings.h \
    src/funcs/FuncBase.h \
    src/funcs/FuncRandomSample.h \
    src/funcs/FuncPlotTextClipboard.h \
    src/funcs/FuncPlotText.h \
    src/funcs/FuncPlotTextFile.h
