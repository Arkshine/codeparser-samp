#-------------------------------------------------
#
# Project created by QtCreator 2014-05-09T22:32:33
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = CodeParser
TEMPLATE = app

win32 {
    INCLUDEPATH += D:\Boost\1.55.0
    LIBS += -LD:\Boost\1.55.0\lib64-msvc-12.0
}

unix {
    LIBS += -lboost_regex
}

SOURCES += main.cpp\
        mainwindow.cpp \
    codeparser.cpp

HEADERS  += mainwindow.h \
    codeparser.h

FORMS    += mainwindow.ui

CONFIG += c++11
