#-------------------------------------------------
#
# Project created by QtCreator 2013-03-04T15:42:50
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = instagramm-grabber
TEMPLATE = app


SOURCES += main.cpp\
        mainwidget.cpp

HEADERS  += mainwidget.h

FORMS    += mainwidget.ui

INCLUDEPATH += ./lib/Headers
LIBS += "d:/my/qjson/build/lib/libqjson.dll.a"

OTHER_FILES += \
    inst_sqr_json.txt
