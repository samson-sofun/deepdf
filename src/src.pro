TARGET = $$PWD/../lib/dpdf

TEMPLATE = lib

CONFIG += c++14

QT = core-private core gui

include($$PWD/3rdparty/pdfium/pdfium.pri)

HEADERS += \
    $$PWD/dpdfiumglobal.h \
    $$PWD/dpdfium.h \
    $$PWD/dpdfiumpage.h \
    dannotation.h \
    dpdfiumdefines.h

SOURCES += \
    $$PWD/dpdfiumglobal.cpp \
    $$PWD/dpdfium.cpp \
    $$PWD/dpdfiumpage.cpp \
    dannotation.cpp
