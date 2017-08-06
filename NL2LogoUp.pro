#-------------------------------------------------
#
# Project created by QtCreator 2017-07-31T08:07:26
#
#-------------------------------------------------

QT       += core gui\
        xml \
        multimedia \
        network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = NL2LogoUp
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
        main.cpp \
        mainwindow.cpp \
    aimlparser.cpp

HEADERS += \
        mainwindow.h \
    aimlparser.h

FORMS += \
        mainwindow.ui

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/libs/ -lmsc_x64
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/libs/ -lmsc_x64d
else:unix: LIBS += -L$$PWD/libs/ -lmsc_x64

INCLUDEPATH += $$PWD/libs
DEPENDPATH += $$PWD/libs

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/libs/libmsc_x64.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/libs/libmsc_x64d.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/libs/msc_x64.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/libs/msc_x64d.lib
else:unix: PRE_TARGETDEPS += $$PWD/libs/libmsc_x64.a
