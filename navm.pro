#-------------------------------------------------
#
# Project created by QtCreator 2018-08-01T11:12:20
#
#-------------------------------------------------

QT       += core gui widgets

TARGET = navm
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
#DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11

SOURCES += \
        main.cpp \
        mainwidget.cpp \
    cube.cpp \
    floor.cpp \
    car.cpp \
    objloader.cpp \
    parts.cpp \
    videosource.cpp

HEADERS += \
        mainwidget.h \
    cube.h \
    floor.h \
    basicobject.h \
    car.h \
    objloader.h \
    parts.h \
    videosource.h


RESOURCES += \
    textures.qrc \
    shaders.qrc

# install
target.path = ~/workspace/bin
INSTALLS += target

