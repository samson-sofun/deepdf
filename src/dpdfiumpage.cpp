#include "dpdfiumpage.h"
#include "public/fpdfview.h"
#include "public/fpdf_text.h"

#include "core/fpdfapi/page/cpdf_page.h"
#include "core/fpdftext/cpdf_textpage.h"
#include "core/fpdfdoc/cpdf_pagelabel.h"

PageHolder::PageHolder(QWeakPointer<FPDF_Document> doc, FPDF_Page *page)
    : m_doc(doc)
    , m_page(page)
    , m_textPage(reinterpret_cast<FPDF_TextPage *>(FPDFText_LoadPage((FPDF_PAGE)page)))
{
}

PageHolder::~PageHolder()
{
    if (m_page)
        FPDF_ClosePage((FPDF_PAGE)m_page);
    if (m_textPage)
        FPDFText_ClosePage((FPDF_TEXTPAGE)m_textPage);
}

DPdfiumPage::DPdfiumPage(QSharedPointer<PageHolder> page, int index)
    : m_pageHolder(page)
    , m_index(index)
{
}

DPdfiumPage::DPdfiumPage(const DPdfiumPage &other)
    : m_pageHolder(other.m_pageHolder)
    , m_index(other.m_index)
{
}

DPdfiumPage &DPdfiumPage::operator=(const DPdfiumPage &other)
{
    m_pageHolder = other.m_pageHolder;
    m_index = other.m_index;
    return *this;
}

DPdfiumPage::~DPdfiumPage()
{

}

qreal DPdfiumPage::width() const
{
    if (!m_pageHolder)
        return -1;
    return FPDF_GetPageWidth((FPDF_PAGE)m_pageHolder.data()->m_page);
}

qreal DPdfiumPage::height() const
{
    if (!m_pageHolder)
        return -1;
    return FPDF_GetPageHeight((FPDF_PAGE)m_pageHolder.data()->m_page);
}


bool DPdfiumPage::isValid() const
{
    return !m_pageHolder.isNull() && !m_pageHolder->m_doc.isNull();
}

int DPdfiumPage::pageIndex() const
{
    return m_index;
}

QImage DPdfiumPage::image(qreal xscale, qreal yscale, qreal x, qreal y, qreal width, qreal height)
{
    if (!isValid())
        return QImage();

    //We need to hold the document while generating the image
    QSharedPointer<FPDF_Document> d = m_pageHolder->m_doc.toStrongRef();

    if (!d)
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

    FPDF_RenderPageBitmap(bitmap, (FPDF_PAGE)m_pageHolder.data()->m_page,
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
    return  FPDFText_CountChars((FPDF_TEXTPAGE)m_pageHolder->m_textPage);
}

QVector<QRectF> DPdfiumPage::getTextRects(int start, int count) const
{
    QVector<QRectF> result;
    std::vector<CFX_FloatRect> pdfiumRects = reinterpret_cast<CPDF_TextPage *>(m_pageHolder->m_textPage)->GetRectArray(start, count);
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
    auto text = reinterpret_cast<CPDF_TextPage *>(m_pageHolder->m_textPage)->GetTextByRect(fxRect);
    return QString::fromWCharArray(text.c_str(), text.GetLength());
}

QString DPdfiumPage::text() const
{
    return text(0, countChars());
}

QString DPdfiumPage::text(int start, int charCount) const
{
    auto text = reinterpret_cast<CPDF_TextPage *>(m_pageHolder->m_textPage)->GetPageText(start, charCount);
    return QString::fromWCharArray(text.c_str(), charCount);
}

QString DPdfiumPage::label() const
{
    CPDF_PageLabel label(reinterpret_cast<CPDF_Document *>(m_pageHolder->m_doc.data()));
    const Optional<WideString> &str = label.GetLabel(pageIndex());
    if (str.has_value())
        return QString::fromWCharArray(str.value().c_str(), str.value().GetLength());
    return QString();
}

