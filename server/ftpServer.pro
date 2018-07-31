#-------------------------------------------------
#
# Project created by QtCreator 2017-12-15T19:48:38
#
#-------------------------------------------------

QT       += core gui
QT       += sql

LIBS += -lpthread -lcrypt

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ftpServer
TEMPLATE = app

DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
        main.cpp \
        ftpserver.cpp \
    server.cpp \
    serverthread.cpp \
    serverconfig.cpp \
    listenthread.cpp \
    usermana.cpp

HEADERS += \
        ftpserver.h \
    server.h \
    serverthread.h \
    serverconfig.h \
    listenthread.h \
    usermana.h

FORMS += \
        ftpserver.ui \
    usermana.ui

LIBS += -L$$OUT_PWD/../common/ -lcommon

INCLUDEPATH += $$PWD/../common
DEPENDPATH += $$PWD/../common

PRE_TARGETDEPS += $$OUT_PWD/../common/libcommon.a

