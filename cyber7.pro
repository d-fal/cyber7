#-------------------------------------------------
#
# Project created by QtCreator 2015-06-06T03:29:18
#
#-------------------------------------------------

QT       += core gui sql network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = cyber7
TEMPLATE = app


SOURCES += main.cpp\
        home.cpp \
    cserial.cpp \
    connectionmanager.cpp \
    database.cpp \
    sessionmanagement.cpp \
    questionpanel.cpp \
    drawvotingstatistics.cpp \
    syncdata.cpp \
    networkmanager.cpp \
    submittedpeople.cpp

HEADERS  += home.h \
    cserial.h \
    connectionmanager.h \
    database.h \
    sessionmanagement.h \
    questionpanel.h \
    drawvotingstatistics.h \
    syncdata.h \
    networkmanager.h \
    submittedpeople.h

FORMS    += home.ui \
    sessionmanagement.ui \
    questionpanel.ui \
    drawvotingstatistics.ui \
    networkmanager.ui \
    submittedpeople.ui

OTHER_FILES += \
    myapp.rc
RC_FILE = myapp.rc
