TARGET = $$PWD/../lib/deepdf

VERSION = 1.0.0.0

TEMPLATE = lib

CONFIG += c++14

DEFINES += BUILD_DEEPDF_LIB

QT = core-private core gui

include($$PWD/3rdparty/pdfium/pdfium.pri)

INCLUDEPATH += $$PWD/../include

INCLUDEPATH += /usr/include/chardet

LIBS += -lchardet

public_headers += \
    $$PWD/../include/dpdfglobal.h \
    $$PWD/../include/dpdfdoc.h \
    $$PWD/../include/dpdfpage.h \
    $$PWD/../include/dpdfannot.h

HEADERS += $$public_headers

SOURCES += \
    $$PWD/dpdfglobal.cpp \
    $$PWD/dpdfdoc.cpp \
    $$PWD/dpdfpage.cpp \
    $$PWD/dpdfannot.cpp

target.path  = /usr/lib

header_files.path = /usr/include

header_files.files = $$public_headers

INSTALLS += target header_files
