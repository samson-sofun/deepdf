#include "dpdfiumpage.h"
#include "dpdfium.h"
#include "dannotation.h"

#include "public/fpdfview.h"
#include "public/fpdf_text.h"
#include "public/fpdf_annot.h"

#include "core/fpdfapi/page/cpdf_page.h"
#include "core/fpdftext/cpdf_textpage.h"
#include "core/fpdfdoc/cpdf_pagelabel.h"

#include <QDebug>

class DPdfiumPagePrivate
{
    friend class DPdfiumPage;
public:
    DPdfiumPagePrivate(DPdfiumDocumentHandler *handler, int index);

    ~DPdfiumPagePrivate();

private:
    FPDF_DOCUMENT m_doc = nullptr;

    FPDF_PAGE m_page = nullptr;

    FPDF_TEXTPAGE m_textPage = nullptr;

    int m_index = -1;

    QList<DAnnotation *> m_dAnnotations;
};

DPdfiumPagePrivate::DPdfiumPagePrivate(DPdfiumDocumentHandler *handler, int index)
{
    m_doc = reinterpret_cast<FPDF_DOCUMENT>(handler);

    m_index = index;

    m_page = FPDF_LoadPage(m_doc, m_index);

    m_textPage = FPDFText_LoadPage(m_page);

    //获取当前注释
    int annotCount = FPDFPage_GetAnnotCount(m_page);

    for (int i = 0; i < annotCount; ++i) {
        FPDF_ANNOTATION annot = FPDFPage_GetAnnot(m_page, i);

        FPDF_ANNOTATION_SUBTYPE subType = FPDFAnnot_GetSubtype(annot);

        int type = DAnnotation::AUnknown;

        if (FPDF_ANNOT_TEXT == subType)
            type = DAnnotation::AText;
        else if (FPDF_ANNOT_LINK == subType)
            type = DAnnotation::AHighlight;
        else if (FPDF_ANNOT_HIGHLIGHT == subType)
            type = DAnnotation::ALink;

        if (DAnnotation::AUnknown != type) {
            FS_RECTF fRectF;
            DAnnotation *dAnnotation = new DAnnotation(type);
            if (FPDFAnnot_GetRect(annot, &fRectF)) {
                dAnnotation->setBoundary(QRectF(fRectF.left, fRectF.top, (fRectF.right - fRectF.left), (fRectF.bottom - fRectF.top)));
            }
            m_dAnnotations.append(dAnnotation);
        }

        FPDFPage_CloseAnnot(annot);
    }
}

DPdfiumPagePrivate::~DPdfiumPagePrivate()
{
    if (m_textPage)
        FPDFText_ClosePage(m_textPage);

    for (DAnnotation *dAnnot : m_dAnnotations) {
        delete dAnnot;
    }

    if (m_page)
        FPDF_ClosePage(m_page);
}

DPdfiumPage::DPdfiumPage(DPdfiumDocumentHandler *handler, int pageIndex)
    : d_ptr(new DPdfiumPagePrivate(handler, pageIndex))
{

}

DPdfiumPage::~DPdfiumPage()
{

}

qreal DPdfiumPage::width() const
{
    return FPDF_GetPageWidth(d_func()->m_page);
}

qreal DPdfiumPage::height() const
{
    return FPDF_GetPageHeight(d_func()->m_page);
}

int DPdfiumPage::pageIndex() const
{
    return d_func()->m_index;
}

QImage DPdfiumPage::image(qreal xscale, qreal yscale, qreal x, qreal y, qreal width, qreal height)
{
    if (nullptr == d_func()->m_doc)
        return QImage();

    QImage image(width * xscale, height * yscale, QImage::Format_RGBA8888);

    if (image.isNull())
        return QImage();
    image.fill(0xFFFFFFFF);

    FPDF_BITMAP bitmap = FPDFBitmap_CreateEx(image.width(), image.height(),
                                             FPDFBitmap_BGRA,
                                             image.scanLine(0), image.bytesPerLine());

    if (bitmap == NULL) {
        return QImage();
    }

    FPDF_RenderPageBitmap(bitmap, d_func()->m_page,
                          0, 0, image.width(), image.height(),
                          0, 0); // no rotation, no flags
    FPDFBitmap_Destroy(bitmap);
    bitmap = NULL;

    for (int i = 0; i < image.height(); i++) {
        uchar *pixels = image.scanLine(i);
        for (int j = 0; j < image.width(); j++) {
            qSwap(pixels[0], pixels[2]);
            pixels += 4;
        }
    }

    return image;
}

int DPdfiumPage::countChars() const
{
    return FPDFText_CountChars(d_func()->m_textPage);
}

QVector<QRectF> DPdfiumPage::getTextRects(int start, int count) const
{
    QVector<QRectF> result;
    std::vector<CFX_FloatRect> pdfiumRects = reinterpret_cast<CPDF_TextPage *>(d_func()->m_textPage)->GetRectArray(start, count);
    result.reserve(pdfiumRects.size());
    for (CFX_FloatRect &rect : pdfiumRects) {
        // QRectF coordinates have their origin point top left instead of bottom left for CFX_FloatRect
        result.push_back({rect.left, height() - rect.top, rect.right - rect.left, rect.top - rect.bottom});
    }
    return result;
}

QString DPdfiumPage::text(const QRectF &rect) const
{
    // QRectF coordinates have their origin point top left instead of bottom left for CFX_FloatRect,
    // so here we reverse the symetry done in getTextRects.
    qreal newBottom = height() - rect.bottom();
    qreal newTop = height() - rect.top();
    CFX_FloatRect fxRect(rect.left(), std::min(newBottom, newTop), rect.right(), std::max(newBottom, newTop));
    auto text = reinterpret_cast<CPDF_TextPage *>(d_func()->m_textPage)->GetTextByRect(fxRect);
    return QString::fromWCharArray(text.c_str(), text.GetLength());
}

QString DPdfiumPage::text() const
{
    return text(0, countChars());
}

QString DPdfiumPage::text(int start, int charCount) const
{
    auto text = reinterpret_cast<CPDF_TextPage *>(d_func()->m_textPage)->GetPageText(start, charCount);
    return QString::fromWCharArray(text.c_str(), charCount);
}

QString DPdfiumPage::label() const
{
    CPDF_PageLabel label(reinterpret_cast<CPDF_Document *>(d_func()->m_doc));
    const Optional<WideString> &str = label.GetLabel(pageIndex());
    if (str.has_value())
        return QString::fromWCharArray(str.value().c_str(), str.value().GetLength());
    return QString();
}

QList<DAnnotation *> DPdfiumPage::annotations()
{
    return d_func()->m_dAnnotations;
}

bool DPdfiumPage::removeAnnotation(DAnnotation *annot)
{
    int index = d_func()->m_dAnnotations.indexOf(annot);

    if (index < 0)
        return false;

    if (!FPDFPage_RemoveAnnot(d_func()->m_page, index))
        return false;

    d_func()->m_dAnnotations.removeOne(annot);

    delete annot;

    emit annotationRemoved(index);

    return true;
}

