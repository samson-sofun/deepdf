#include "dpdfdoc.h"
#include "dpdfpage.h"
#include "dpdfannot.h"

#include "public/fpdfview.h"
#include "public/fpdf_text.h"
#include "public/fpdf_annot.h"
#include "public/fpdf_doc.h"

#include "core/fpdfapi/page/cpdf_page.h"
#include "core/fpdftext/cpdf_textpage.h"
#include "core/fpdfdoc/cpdf_pagelabel.h"
#include "core/fpdfdoc/cpdf_linklist.h"

#include <QDebug>

class DPdfPagePrivate
{
    friend class DPdfPage;
public:
    DPdfPagePrivate(DPdfDocHandler *handler, int index);

    ~DPdfPagePrivate();

private:
    FPDF_DOCUMENT m_doc = nullptr;

    FPDF_PAGE m_page = nullptr;

    FPDF_TEXTPAGE m_textPage = nullptr;

    int m_index = -1;

    QList<DPdfAnnot *> m_dAnnots;
};

DPdfPagePrivate::DPdfPagePrivate(DPdfDocHandler *handler, int index)
{
    m_doc = reinterpret_cast<FPDF_DOCUMENT>(handler);

    m_index = index;

    m_page = FPDF_LoadPage(m_doc, m_index);

    m_textPage = FPDFText_LoadPage(m_page);

    //获取当前注释
    int annotCount = FPDFPage_GetAnnotCount(m_page);

    for (int i = 0; i < annotCount; ++i) {
        FPDF_ANNOTATION annot = FPDFPage_GetAnnot(m_page, i);

        qDebug() << i << ":" << FPDFPage_GetAnnotIndex(m_page, annot);

        FPDF_ANNOTATION_SUBTYPE subType = FPDFAnnot_GetSubtype(annot);

        DPdfAnnot::AnnotType type = DPdfAnnot::AUnknown;

        if (FPDF_ANNOT_TEXT == subType)
            type = DPdfAnnot::AText;
        else if (FPDF_ANNOT_HIGHLIGHT == subType)
            type = DPdfAnnot::AHighlight;

        DPdfAnnot *dAnnot = new DPdfAnnot(type);
        if (DPdfAnnot::AUnknown != type) {
            FS_RECTF rectF;
            if (FPDFAnnot_GetRect(annot, &rectF)) {
                dAnnot->setBoundary(QRectF(rectF.left, rectF.top, (rectF.right - rectF.left), (rectF.bottom - rectF.top)));
            }
        }
        m_dAnnots.append(dAnnot);

        FPDFPage_CloseAnnot(annot);
    }
}

DPdfPagePrivate::~DPdfPagePrivate()
{
    if (m_textPage)
        FPDFText_ClosePage(m_textPage);

    for (DPdfAnnot *dAnnot : m_dAnnots) {
        delete dAnnot;
    }

    if (m_page)
        FPDF_ClosePage(m_page);
}

DPdfPage::DPdfPage(DPdfDocHandler *handler, int pageIndex)
    : d_ptr(new DPdfPagePrivate(handler, pageIndex))
{

}

DPdfPage::~DPdfPage()
{

}

qreal DPdfPage::width() const
{
    return FPDF_GetPageWidth(d_func()->m_page);
}

qreal DPdfPage::height() const
{
    return FPDF_GetPageHeight(d_func()->m_page);
}

int DPdfPage::pageIndex() const
{
    return d_func()->m_index;
}

QImage DPdfPage::image(qreal xscale, qreal yscale, qreal x, qreal y, qreal width, qreal height)
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

int DPdfPage::countChars() const
{
    return FPDFText_CountChars(d_func()->m_textPage);
}

QRectF DPdfPage::getTextRect(int start) const
{
    QRectF result;
    std::vector<CFX_FloatRect> pdfiumRects = reinterpret_cast<CPDF_TextPage *>(d_func()->m_textPage)->GetRectArray(start, 1);
    for (CFX_FloatRect &rect : pdfiumRects) {
        rect.Normalize();
        result = QRectF(rect.left, height() - rect.top, rect.right - rect.left, rect.top - rect.bottom);
    }
    return result;
}

QString DPdfPage::text(const QRectF &rect) const
{
    qreal newBottom = height() - rect.bottom();
    qreal newTop = height() - rect.top();
    CFX_FloatRect fxRect(rect.left(), std::min(newBottom, newTop), rect.right(), std::max(newBottom, newTop));
    auto text = reinterpret_cast<CPDF_TextPage *>(d_func()->m_textPage)->GetTextByRect(fxRect);
    return QString::fromWCharArray(text.c_str(), text.GetLength());
}

QString DPdfPage::text(int start, int charCount) const
{
    auto text = reinterpret_cast<CPDF_TextPage *>(d_func()->m_textPage)->GetPageText(start, charCount);
    return QString::fromWCharArray(text.c_str(), charCount);
}

QString DPdfPage::label() const
{
    CPDF_PageLabel label(reinterpret_cast<CPDF_Document *>(d_func()->m_doc));
    const Optional<WideString> &str = label.GetLabel(pageIndex());
    if (str.has_value())
        return QString::fromWCharArray(str.value().c_str(), str.value().GetLength());
    return QString();
}

QList<DPdfAnnot *> DPdfPage::annots()
{
    QList<DPdfAnnot *> annots;
    for (DPdfAnnot *annot : d_func()->m_dAnnots) {
        if (annot->type() == DPdfAnnot::AHighlight || annot->type() == DPdfAnnot::AText)
            annots.append(annot);
    }

    return annots;
}

DPdfPage::Link DPdfPage::getLinkAtPoint(qreal x, qreal y)
{
    Link link;
    const FPDF_LINK &flink = FPDFLink_GetLinkAtPoint(d_func()->m_page, x, height() - y);
    CPDF_Link cLink(reinterpret_cast<CPDF_Dictionary *>(flink));
    if (cLink.GetDict() == nullptr)
        return link;

    CPDF_Document *pDoc = reinterpret_cast<CPDF_Document *>(d_func()->m_doc);
    const CPDF_Action &cAction = cLink.GetAction();
    const CPDF_Dest &dest = cAction.GetDest(pDoc);

    link.nIndex = dest.GetDestPageIndex(pDoc);
    if (cAction.GetType() == CPDF_Action::URI) {
        const ByteString &bUrl = cAction.GetURI(pDoc);
        link.urlpath = QString::fromUtf8(bUrl.c_str(), bUrl.GetLength());
    } else {
        const WideString &wFilepath = cAction.GetFilePath();
        link.urlpath = QString::fromWCharArray(wFilepath.c_str(), wFilepath.GetLength());
    }
    return link;
}

bool DPdfPage::createAnnot(int annotType, QColor color, QRectF boudary)
{
    FPDF_ANNOTATION_SUBTYPE subType = FPDF_ANNOT_UNKNOWN;

    if (DPdfAnnot::AText == annotType)
        subType = FPDF_ANNOT_TEXT;
    else if (DPdfAnnot::AHighlight == annotType)
        subType = FPDF_ANNOT_HIGHLIGHT;
    else
        return false;

    FPDF_ANNOTATION annot = FPDFPage_CreateAnnot(d_func()->m_page, subType);

    if (color.isValid() && !FPDFAnnot_SetColor(annot, FPDFANNOT_COLORTYPE_Color, color.red(), color.green(), color.blue(), color.alpha())) {
        FPDFPage_CloseAnnot(annot);
        return false;
    }

    FS_RECTF rectF;
    rectF.left = boudary.x();
    rectF.top = boudary.y();
    rectF.left = boudary.x() + boudary.width();
    rectF.bottom = boudary.y() + boudary.height();

    if (boudary.isValid() && !FPDFAnnot_SetRect(annot, &rectF)) {
        FPDFPage_CloseAnnot(annot);
        return false;
    }

    FPDFPage_CloseAnnot(annot);

    DPdfAnnot *dAnnot = new DPdfAnnot((DPdfAnnot::AnnotType)annotType);

    dAnnot->setBoundary(boudary);

    d_func()->m_dAnnots.append(dAnnot);

    emit annotAdded(dAnnot);

    return true;
}

bool DPdfPage::updateAnnot(DPdfAnnot *dAnnot, QColor color, QRectF boudary)
{
    int index = d_func()->m_dAnnots.indexOf(dAnnot);

    FPDF_ANNOTATION annot = FPDFPage_GetAnnot(d_func()->m_page, index);

    if (color.isValid() && !FPDFAnnot_SetColor(annot, FPDFANNOT_COLORTYPE_Color, color.red(), color.green(), color.blue(), color.alpha())) {
        FPDFPage_CloseAnnot(annot);
        return false;
    }

    FS_RECTF rectF;
    rectF.left = boudary.x();
    rectF.top = boudary.y();
    rectF.left = boudary.x() + boudary.width();
    rectF.bottom = boudary.y() + boudary.height();

    if (boudary.isValid() && !FPDFAnnot_SetRect(annot, &rectF)) {
        FPDFPage_CloseAnnot(annot);
        return false;
    }

    FPDFPage_CloseAnnot(annot);

    emit annotUpdated(dAnnot);

    return true;
}

bool DPdfPage::removeAnnot(DPdfAnnot *dAnnot)
{
    int index = d_func()->m_dAnnots.indexOf(dAnnot);

    if (index < 0)
        return false;

    if (!FPDFPage_RemoveAnnot(d_func()->m_page, index))
        return false;

    d_func()->m_dAnnots.removeOne(dAnnot);

    emit annotRemoved(dAnnot);

    delete dAnnot;

    return true;
}
