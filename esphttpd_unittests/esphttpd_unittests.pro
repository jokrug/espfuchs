TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.c \

HEADERS += \


INCLUDEPATH += \
    ../include \
    ../libesphttpd/include \
    .

DISTFILES += \
    ../* \
    ../driver/* \
    ../html/* \
    ../libesphttpd/* \
    ../user/* \




