#-------------------------------------------------
#
# Project created by QtCreator 2016-05-17T21:55:59
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = DM3U
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    parserm3u.cpp \
    backtracking.cpp \
    channels.cpp \
    m3utemplates.cpp \
    export.cpp \
    advancedtablew.cpp \
    groups_w.cpp \
    groups.cpp \
    qtableviewext.cpp

HEADERS  += mainwindow.h \
    parserm3u.h \
    backtracking.h \
    channels.h \
    m3utemplates.h \
    export.h \
    advancedtablew.h \
    groups.h \
    qtableviewext.h

FORMS    += mainwindow.ui \
    channels.ui \
    groups.ui

QMAKE_CXXFLAGS += -std=c++11
