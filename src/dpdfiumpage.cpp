#include "dpdfiumpage.h"
#include "dpdfium.h"

#include "public/fpdfview.h"
#include "public/fpdf_text.h"
#include "public/fpdf_annot.h"

#include "core/fpdfapi/page/cpdf_page.h"
#include "core/fpdftext/cpdf_textpage.h"
#include "core/fpdfdoc/cpdf_pagelabel.h"

#include <QDebug>

class DPdfiumPagePrivate
{
public:
    DPdfiumPagePrivate(DPdfiumDocumentHandler *handler,int index);

    ~DPdfiumPagePrivate();

private:
    FPDF_DOCUMENT m_doc = nullptr;
    FPDF_PAGE m_page = nullptr;
    FPDF_TEXTPAGE m_textPage = nullptr;
    int m_index = -1;
    QList<FPDF_ANNOTATION> m_annotations;
};

DPdfiumPagePrivate::DPdfiumPagePrivate(DPdfiumDocumentHandler *handler, int index)
{
    m_doc = reinterpret_cast<FPDF_DOCUMENT>(handler);

    m_index = index;

    m_page = FPDF_LoadPage((FPDF_DOCUMENT)handler, m_index);

    m_textPage = FPDFText_LoadPage((FPDF_PAGE)m_page);

    int annotCount = FPDFPage_GetAnnotCount(m_page);

    for(int i = 0;i < annotCount; ++i)
    {
         m_annotations.append(FPDFPage_GetAnnot(m_page,i));
     }
}

DPdfiumPagePrivate::~DPdfiumPagePrivate()
{
    if (m_textPage)
        FPDFText_ClosePage(m_textPage);

    for(FPDF_ANNOTATION annot:m_annotations)
    {
        FPDFPage_CloseAnnot(annot);
    }

    if (m_page)
        FPDF_ClosePage(m_page);
}

DPdfiumPage::DPdfiumPage(DPdfiumDocumentHandler *handler, int pageIndex)
    : m_private(new DPdfiumPagePrivate(handler,pageIndex))
    , m_index(pageIndex)
{

}

DPdfiumPage::~DPdfiumPage()
{

}

qreal DPdfiumPage::width() const
{
    if (!m_private)
        return -1;

    return FPDF_GetPageWidth(m_private.data()->m_page);
}

qreal DPdfiumPage::height() const
{
    if (!m_private)
        return -1;

    return FPDF_GetPageHeight(m_private.data()->m_page);
}


bool DPdfiumPage::isValid() const
{
    return !m_private.isNull() && (m_private->m_doc != nullptr);
}

int DPdfiumPage::pageIndex() const
{
    return m_index;
}

QImage DPdfiumPage::image(qreal xscale, qreal yscale, qreal x, qreal y, qreal width, qreal height)
{
    if (!isValid())
        return QImage();

    if (nullptr == m_private->m_doc)
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

    FPDF_RenderPageBitmap(bitmap, (FPDF_PAGE)m_private.data()->m_page,
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
    return  FPDFText_CountChars((FPDF_TEXTPAGE)m_private->m_textPage);
}

QVector<QRectF> DPdfiumPage::getTextRects(int start, int count) const
{
    QVector<QRectF> result;
    std::vector<CFX_FloatRect> pdfiumRects = reinterpret_cast<CPDF_TextPage *>(m_private->m_textPage)->GetRectArray(start, count);
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
    auto text = reinterpret_cast<CPDF_TextPage *>(m_private->m_textPage)->GetTextByRect(fxRect);
    return QString::fromWCharArray(text.c_str(), text.GetLength());
}

QString DPdfiumPage::text() const
{
    return text(0, countChars());
}

QString DPdfiumPage::text(int start, int charCount) const
{
    auto text = reinterpret_cast<CPDF_TextPage *>(m_private->m_textPage)->GetPageText(start, charCount);
    return QString::fromWCharArray(text.c_str(), charCount);
}

QString DPdfiumPage::label() const
{
    CPDF_PageLabel label(reinterpret_cast<CPDF_Document *>(m_private->m_doc));
    const Optional<WideString> &str = label.GetLabel(pageIndex());
    if (str.has_value())
        return QString::fromWCharArray(str.value().c_str(), str.value().GetLength());
    return QString();
}

QList<DAnnotation *> DPdfiumPage::annotations()
{
    QList<DAnnotation *> annotations();
    return m_private->m_annotations;
}

bool DPdfiumPage::deleteAnnotation(int index)
{
    return FPDFPage_RemoveAnnot(m_private->m_page,index);
}

