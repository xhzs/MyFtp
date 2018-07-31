#-------------------------------------------------
#
# Project created by QtCreator 2017-12-15T19:48:16
#
#-------------------------------------------------

QT       += core gui

LIBS += -lpthread

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ftpClient
TEMPLATE = app

DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
        main.cpp \
        ftpclient.cpp \
    client.cpp \
    clientthread.cpp \
    infothread.cpp

HEADERS += \
        ftpclient.h\
    client.h \
    clientthread.h \
    infothread.h


FORMS += \
        ftpclient.ui

LIBS += -L$$OUT_PWD/../common/ -lcommon

INCLUDEPATH += $$PWD/../common
DEPENDPATH += $$PWD/../common

PRE_TARGETDEPS += $$OUT_PWD/../common/libcommon.a
