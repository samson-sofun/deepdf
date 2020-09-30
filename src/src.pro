TARGET = $$PWD/../lib/dpdf

TEMPLATE = lib

CONFIG += c++11

QT = core-private core gui

include($$PWD/3rdparty/pdfium/pdfium.pri)

HEADERS += \
    $$PWD/dpdfglobal.h \
    $$PWD/dpdfdoc.h \
    $$PWD/dpdfpage.h \
    $$PWD/dpdfiumdefines.h \
    $$PWD/dpdfannot.h

SOURCES += \
    $$PWD/dpdfglobal.cpp \
    $$PWD/dpdfdoc.cpp \
    $$PWD/dpdfpage.cpp \
    $$PWD/dpdfannot.cpp
