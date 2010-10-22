#-------------------------------------------------
#
# Project created by QtCreator 2010-04-13T22:13:00
#
#-------------------------------------------------
INCLUDEPATH += /home/royce/QExtSerial/src
LIBS += -L'/home/royce/QExtSerial/src/build/'
CONFIG(debug, debug|release):LIBS += -lqextserialportd
else:LIBS += -lqextserialport

TARGET = AdminRFID
TEMPLATE = app


SOURCES += main.cpp\
        adminrfid.cpp

HEADERS  += adminrfid.h

FORMS    += adminrfid.ui
