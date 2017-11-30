# -------------------------------------------------
# Project created by QtCreator 2010-04-03T15:28:20
# -------------------------------------------------

QT += network xml sql serialport
QT -= gui
TARGET = AccessServer
CONFIG += console
CONFIG -= app_bundle
TEMPLATE = app

SOURCES += main.cpp \
    busmngr.cpp \
    ASCIIProtocol.cpp

HEADERS += busmngr.h \
    ASCIIProtocol.h



