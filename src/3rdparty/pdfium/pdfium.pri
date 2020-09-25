SOURCE_DIR = $$PWD/pdfium

INCLUDEPATH += $$SOURCE_DIR

DEFINES += __QT__ \
    OPJ_STATIC \
    PNG_PREFIX \
    PNG_USE_READ_MACROS \
    BUILD_DEEPIN_PDFIUM_LIB \
    FT2_BUILD_LIBRARY

DEFINES +=  USE_SYSTEM_LIBJPEG \
            USE_SYSTEM_ZLIB \
            USE_SYSTEM_LIBPNG \
            USE_SYSTEM_FREETYPE \
            USE_SYSTEM_ICUUC

if(QMAKE_HOST.arch, x86_64) {
    DEFINES += "_FX_CPU_=_FX_X64_"
} else {
    DEFINES += "_FX_CPU_=_FX_X32_"
}

include(fx_freetype.pri)
include(fpdfsdk.pri)
include(core.pri)
include(fx_libopenjpeg.pri)
include(fx_agg.pri)
include(fxjs.pri)
include(fx_lcms2.pri)
include(fx_skia.pri)
include(fx_base.pri)

HEADERS += \
    $$PWD/pdfium/public/cpp/fpdf_deleters.h \
    $$PWD/pdfium/public/cpp/fpdf_scopers.h \
    $$PWD/pdfium/public/fpdf_annot.h \
    $$PWD/pdfium/public/fpdf_attachment.h \
    $$PWD/pdfium/public/fpdf_catalog.h \
    $$PWD/pdfium/public/fpdf_dataavail.h \
    $$PWD/pdfium/public/fpdf_doc.h \
    $$PWD/pdfium/public/fpdf_edit.h \
    $$PWD/pdfium/public/fpdf_ext.h \
    $$PWD/pdfium/public/fpdf_flatten.h \
    $$PWD/pdfium/public/fpdf_formfill.h \
    $$PWD/pdfium/public/fpdf_fwlevent.h \
    $$PWD/pdfium/public/fpdf_javascript.h \
    $$PWD/pdfium/public/fpdf_ppo.h \
    $$PWD/pdfium/public/fpdf_progressive.h \
    $$PWD/pdfium/public/fpdf_save.h \
    $$PWD/pdfium/public/fpdf_searchex.h \
    $$PWD/pdfium/public/fpdf_signature.h \
    $$PWD/pdfium/public/fpdf_structtree.h \
    $$PWD/pdfium/public/fpdf_sysfontinfo.h \
    $$PWD/pdfium/public/fpdf_text.h \
    $$PWD/pdfium/public/fpdf_thumbnail.h \
    $$PWD/pdfium/public/fpdf_transformpage.h \
    $$PWD/pdfium/public/fpdfview.h
