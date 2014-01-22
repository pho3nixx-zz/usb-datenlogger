#-------------------------------------------------
#
# Project created by QtCreator 2013-08-26T20:11:04
#
#-------------------------------------------------
CONFIG += serialport
CONFIG += qwt
INCLUDEPATH +="/usr/local/qwt-6.1.0/include"
QT       += core gui

TARGET = CommanderDataLogV2
TEMPLATE = app


SOURCES += main.cpp\
        commanderdatalog.cpp

HEADERS  += commanderdatalog.h

FORMS    += commanderdatalog.ui

LIBS += -L/usr/local/qwt-6.1.0/lib -lqwt

OTHER_FILES += \
    ListOfCommands.txt

