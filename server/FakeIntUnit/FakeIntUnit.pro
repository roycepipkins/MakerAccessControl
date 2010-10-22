# -------------------------------------------------
# Project created by QtCreator 2010-05-05T15:46:46
# -------------------------------------------------
TARGET = FakeIntUnit
TEMPLATE = app
unix { 
    INCLUDEPATH += /home/royce/QExtSerial/src
    LIBS += -L'/home/royce/QExtSerial/src/build/'
}
win32 { 
    INCLUDEPATH += C:\Qt\QExtSerial\src
    LIBS += -LC:\Qt\QExtSerial\src\build
}

SOURCES += main.cpp \
    mainwindow.cpp
HEADERS += mainwindow.h
FORMS += mainwindow.ui
unix {
    CONFIG(debug, debug|release):LIBS += -lqextserialportd
    else:LIBS += -lqextserialport
}
win32 {
    CONFIG(debug, debug|release):LIBS += -lqextserialportd1
    else:LIBS += -lqextserialport1
}


SOURCES += ASCIIProtocol.cpp
HEADERS += ASCIIProtocol.h
