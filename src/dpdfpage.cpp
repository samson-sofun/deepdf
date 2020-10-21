#include "dpdfdoc.h"
#include "dpdfpage.h"
#include "dpdfannot.h"

#include "public/fpdfview.h"
#include "public/fpdf_text.h"
#include "public/fpdf_annot.h"
#include "public/fpdf_doc.h"

#include "core/fpdfapi/page/cpdf_page.h"
#include "core/fpdftext/cpdf_textpage.h"
#include "core/fpdfdoc/cpdf_linklist.h"

#include <QDebug>

class DPdfPagePrivate
{
    friend class DPdfPage;
public:
    DPdfPagePrivate(DPdfDocHandler *handler, int index);

    ~DPdfPagePrivate();

public:
    void loadPage();

    void loadTextPage();

private:
    void loadAnnots();

private:
    FPDF_DOCUMENT m_doc = nullptr;

    int m_annotInit = false;

    int m_index = -1;

    double m_width = 0;

    qreal m_height = 0;

    FPDF_PAGE m_page = nullptr;

    FPDF_TEXTPAGE m_textPage = nullptr;

    QList<DPdfAnnot *> m_dAnnots;
};

DPdfPagePrivate::DPdfPagePrivate(DPdfDocHandler *handler, int index)
{
    m_doc = reinterpret_cast<FPDF_DOCUMENT>(handler);

    m_index = index;

    FPDF_GetPageSizeByIndex(m_doc, index, &m_width, &m_height);
}

DPdfPagePrivate::~DPdfPagePrivate()
{
    if (m_textPage)
        FPDFText_ClosePage(m_textPage);

    if (m_page)
        FPDF_ClosePage(m_page);

    qDeleteAll(m_dAnnots);
}

void DPdfPagePrivate::loadPage()
{
    if (nullptr == m_page)
        m_page = FPDF_LoadPage(m_doc, m_index);
}

void DPdfPagePrivate::loadTextPage()
{
    loadPage();

    if (nullptr == m_textPage)
        m_textPage = FPDFText_LoadPage(m_page);
}

void DPdfPagePrivate::loadAnnots()
{
    if (m_annotInit)
        return;

    //使用临时page，不完全加载,防止刚开始消耗时间过长
    FPDF_PAGE page = FPDF_LoadNoParsePage(m_doc, m_index);

    //获取当前注释
    int annotCount = FPDFPage_GetAnnotCount(page);

    for (int i = 0; i < annotCount; ++i) {
        FPDF_ANNOTATION annot = FPDFPage_GetAnnot(page, i);

        FPDF_ANNOTATION_SUBTYPE subType = FPDFAnnot_GetSubtype(annot);

        DPdfAnnot::AnnotType type = DPdfAnnot::AUnknown;

        if (FPDF_ANNOT_TEXT == subType)
            type = DPdfAnnot::AText;
        else if (FPDF_ANNOT_HIGHLIGHT == subType)
            type = DPdfAnnot::AHighlight;
        else if (FPDF_ANNOT_LINK == subType)
            type = DPdfAnnot::ALink;

//        @@@@@@@@  "Border"
//        @@@@@@@@  "C"
//        @@@@@@@@  "CA"
//        @@@@@@@@  "Contents"
//        @@@@@@@@  "F"
//        @@@@@@@@  "M"
//        @@@@@@@@  "NM"
//        @@@@@@@@  "P"
//        @@@@@@@@  "Rect"
//        @@@@@@@@  "Subtype"
//        @@@@@@@@  "T"
//        @@@@@@@@  "Type"

//    |----------------------------------|
//    |--------right-------              |
//    |                                  |
//    |----left----........              |
//    |            |.     .              |
//    |            |. . . .              |
//    |           top     |bottom        |
//    |            |      |              |
//    |----------------------------------|

        qreal pageHeight = m_height;
        if (DPdfAnnot::AText == type) {
            DPdfTextAnnot *dAnnot = new DPdfTextAnnot;

            //获取位置
            FS_RECTF rectF;
            if (FPDFAnnot_GetRect(annot, &rectF)) {//注释图标为20x20
                QRectF annorectF(static_cast<qreal>(rectF.left), pageHeight - static_cast<qreal>(rectF.top), 20, 20);
                dAnnot->setRectF(annorectF);

                FS_RECTF newrectf;
                newrectf.left = static_cast<float>(annorectF.left());
                newrectf.top = static_cast<float>(pageHeight - annorectF.top());
                newrectf.right = static_cast<float>(annorectF.right());
                newrectf.bottom = static_cast<float>(pageHeight - annorectF.bottom());
                FPDFAnnot_SetRect(annot, &newrectf);
            }

            ulong quadCount = FPDFAnnot_CountAttachmentPoints(annot);
            QList<QRectF> list;
            for (ulong i = 0; i < quadCount; ++i) {
                FS_QUADPOINTSF quad;
                if (!FPDFAnnot_GetAttachmentPoints(annot, i, &quad))
                    continue;

                QRectF rectF;
                rectF.setX(static_cast<double>(quad.x1));
                rectF.setY(pageHeight - static_cast<double>(quad.y1));
                rectF.setWidth(static_cast<double>(quad.x2 - quad.x1));
                rectF.setHeight(static_cast<double>(quad.y1 - quad.y3));
                list.append(rectF);
            }

            //获取文本
            FPDF_WCHAR buffer[1024];
            FPDFAnnot_GetStringValue(annot, "Contents", buffer, 1024);
            dAnnot->m_text = QString::fromUtf16(buffer);

            m_dAnnots.append(dAnnot);
        } else if (DPdfAnnot::AHighlight == type) {
            DPdfHightLightAnnot *dAnnot = new DPdfHightLightAnnot;
            //获取颜色
            unsigned int r = 0;
            unsigned int g = 0;
            unsigned int b = 0;
            unsigned int a = 255;
            if (FPDFAnnot_GetColor(annot, FPDFANNOT_COLORTYPE_Color, &r, &g, &b, &a)) {
                dAnnot->setColor(QColor(static_cast<int>(r), static_cast<int>(g), static_cast<int>(b), static_cast<int>(a)));
            }

            //获取区域
            ulong quadCount = FPDFAnnot_CountAttachmentPoints(annot);
            QList<QRectF> list;
            for (ulong i = 0; i < quadCount; ++i) {
                FS_QUADPOINTSF quad;
                if (!FPDFAnnot_GetAttachmentPoints(annot, i, &quad))
                    continue;

                QRectF rectF;
                rectF.setX(static_cast<double>(quad.x1));
                rectF.setY(pageHeight - static_cast<double>(quad.y1));
                rectF.setWidth(static_cast<double>(quad.x2 - quad.x1));
                rectF.setHeight(static_cast<double>(quad.y1 - quad.y3));

                list.append(rectF);
            }
            dAnnot->setBoundaries(list);

            //获取文本
            FPDF_WCHAR buffer[1024];
            FPDFAnnot_GetStringValue(annot, "Contents", buffer, 1024);
            dAnnot->m_text = QString::fromUtf16(buffer);

            m_dAnnots.append(dAnnot);
        } else if (DPdfAnnot::ALink == type) {
            DPdfLinkAnnot *dAnnot = new DPdfLinkAnnot;

            FPDF_LINK link = FPDFAnnot_GetLink(annot);

            FPDF_ACTION action = FPDFLink_GetAction(link);

            unsigned long type = FPDFAction_GetType(action);

            //获取位置
            FS_RECTF rectF;
            if (FPDFAnnot_GetRect(annot, &rectF)) {//注释图标为20x20
                QRectF annorectF(static_cast<qreal>(rectF.left),
                                 pageHeight - static_cast<qreal>(rectF.top),
                                 static_cast<qreal>(rectF.right) - static_cast<qreal>(rectF.left),
                                 static_cast<qreal>(rectF.top) - static_cast<qreal>(rectF.bottom));
                dAnnot->setRectF(annorectF);
            }

            //获取类型
            if (PDFACTION_URI == type) {
                char uri[256] = {0};
                unsigned long lenth = FPDFAction_GetURIPath(m_doc, action, uri, 256);
                if (0 != lenth) {
                    dAnnot->setUrl(uri);
                }
                dAnnot->setLinkType(DPdfLinkAnnot::Uri);

            } else if (PDFACTION_REMOTEGOTO == type) {
                char filePath[256] = {0};
                unsigned long lenth = FPDFAction_GetFilePath(action, filePath, 256);
                if (0 != lenth) {
                    dAnnot->setFilePath(filePath);
                }

                dAnnot->setLinkType(DPdfLinkAnnot::RemoteGoTo);
            } else if (PDFACTION_GOTO == type || PDFACTION_UNSUPPORTED == type) { //跳转到文档某处
                FPDF_DEST dest = FPDFAction_GetDest(m_doc, action);

                int index = FPDFDest_GetDestPageIndex(m_doc, dest);

                FPDF_BOOL hasX = false;
                FPDF_BOOL hasY = false;
                FPDF_BOOL hasZ = false;
                FS_FLOAT x = 0;
                FS_FLOAT y = 0;
                FS_FLOAT z = 0;

                bool result = FPDFDest_GetLocationInPage(dest, &hasX, &hasY, &hasZ, &x, &y, &z);

                if (result)
                    dAnnot->setPage(index, hasX ? x : 0, hasY ? y : 0);

                dAnnot->setLinkType(DPdfLinkAnnot::Goto);
            }

            m_dAnnots.append(dAnnot);
        } else {
            //其他类型 用于占位 对应索引
            DPdfUnknownAnnot *dAnnot = new DPdfUnknownAnnot;

            m_dAnnots.append(dAnnot);
        }
        FPDFPage_CloseAnnot(annot);
    }

    FPDF_ClosePage(page);

    m_annotInit = true;
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
    return d_func()->m_width;
}

qreal DPdfPage::height() const
{
    return d_func()->m_height;
}

int DPdfPage::pageIndex() const
{
    return d_func()->m_index;
}

QImage DPdfPage::image(qreal scale)
{
    if (nullptr == d_func()->m_doc)
        return QImage();

    d_func()->loadPage();

    int scaleWidth = static_cast<int>(width() * scale) ;
    int scaleHeight = static_cast<int>(height() * scale);

    QImage image(scaleWidth, scaleHeight, QImage::Format_RGBA8888);

    image.fill(0xFFFFFFFF);

    if (image.isNull())
        return QImage();

    FPDF_BITMAP bitmap = FPDFBitmap_CreateEx(image.width(), image.height(), FPDFBitmap_BGRA, image.scanLine(0), image.bytesPerLine());

    if (bitmap == nullptr) {
        return QImage();
    }

    FPDF_RenderPageBitmap(bitmap, d_func()->m_page, 0, 0, scaleWidth, scaleHeight, scaleWidth, scaleHeight, 0, FPDF_ANNOT);

    FPDFBitmap_Destroy(bitmap);

    for (int i = 0; i < image.height(); i++) {
        uchar *pixels = image.scanLine(i);
        for (int j = 0; j < image.width(); j++) {
            qSwap(pixels[0], pixels[2]);
            pixels += 4;
        }
    }

    return image;
}

QImage DPdfPage::image(qreal xscale, qreal yscale, qreal x, qreal y, qreal width, qreal height)
{
    if (nullptr == d_func()->m_doc)
        return QImage();

    d_func()->loadPage();

    QImage image(width, height, QImage::Format_RGBA8888);

    if (image.isNull())
        return QImage();

    image.fill(0xFFFFFFFF);

    FPDF_BITMAP bitmap = FPDFBitmap_CreateEx(image.width(), image.height(),
                                             FPDFBitmap_BGRA,
                                             image.scanLine(0), image.bytesPerLine());

    if (bitmap == nullptr) {
        return QImage();
    }

    FPDF_RenderPageBitmap(bitmap, d_func()->m_page,
                          x, y, width, height,
                          xscale * this->width(), yscale * this->height(),
                          0, FPDF_ANNOT);
    FPDFBitmap_Destroy(bitmap);
    bitmap = nullptr;

    for (int i = 0; i < image.height(); i++) {
        uchar *pixels = image.scanLine(i);
        for (int j = 0; j < image.width(); j++) {
            qSwap(pixels[0], pixels[2]);
            pixels += 4;
        }
    }

    return image;
}

int DPdfPage::countChars()
{
    d_func()->loadTextPage();

    return FPDFText_CountChars(d_func()->m_textPage);
}

QVector<QRectF> DPdfPage::getTextRects(int start, int charCount)
{
    d_func()->loadTextPage();

    QVector<QRectF> result;
    const std::vector<CFX_FloatRect> &pdfiumRects = reinterpret_cast<CPDF_TextPage *>(d_func()->m_textPage)->GetRectArray(start, charCount);
    result.reserve(pdfiumRects.size());
    for (const CFX_FloatRect &rect : pdfiumRects) {
        result.push_back({rect.left, height() - rect.top, rect.right - rect.left, rect.top - rect.bottom});
    }
    return result;
}

bool DPdfPage::getTextRect(int start, QRectF &textrect)
{
    d_func()->loadTextPage();

    if (FPDFText_GetUnicode(d_func()->m_textPage, start) == L' ') {
        textrect = QRectF();
        return true;
    }

    FS_RECTF rect;
    if (FPDFText_GetLooseCharBox(d_func()->m_textPage, start, &rect)) {
        textrect = QRectF(rect.left, height() - rect.top, rect.right - rect.left, rect.top - rect.bottom);
        return true;
    }

    return  false;
}

QString DPdfPage::text(const QRectF &rect)
{
    d_func()->loadTextPage();

    qreal newBottom = height() - rect.bottom();
    qreal newTop = height() - rect.top();
    CFX_FloatRect fxRect(rect.left(), std::min(newBottom, newTop), rect.right(), std::max(newBottom, newTop));
    auto text = reinterpret_cast<CPDF_TextPage *>(d_func()->m_textPage)->GetTextByRect(fxRect);
    return QString::fromWCharArray(text.c_str(), text.GetLength());
}

QString DPdfPage::text(int start, int charCount)
{
    d_func()->loadTextPage();

    auto text = reinterpret_cast<CPDF_TextPage *>(d_func()->m_textPage)->GetPageText(start, charCount);
    return QString::fromWCharArray(text.c_str(), text.GetLength());
}

DPdfPage::Link DPdfPage::getLinkAtPoint(qreal x, qreal y)
{
    d_func()->loadPage();

    Link link;
    const FPDF_LINK &flink = FPDFLink_GetLinkAtPoint(d_func()->m_page, x, height() - y);
    CPDF_Link cLink(reinterpret_cast<CPDF_Dictionary *>(flink));
    if (cLink.GetDict() == nullptr)
        return link;

    CPDF_Document *pDoc = reinterpret_cast<CPDF_Document *>(d_func()->m_doc);
    const CPDF_Action &cAction = cLink.GetAction();
    const CPDF_Dest &dest = cAction.GetDest(pDoc);

    bool hasx = false, hasy = false, haszoom = false;
    float offsetx = 0.0, offsety = 0.0, z = 0.0;
    dest.GetXYZ(&hasx, &hasy, &haszoom, &offsetx, &offsety, &z);
    link.left = offsetx;
    link.top = offsety;

    link.nIndex = dest.GetDestPageIndex(pDoc);
    if (cAction.GetType() == CPDF_Action::URI) {
        const ByteString &bUrl = cAction.GetURI(pDoc);
        link.urlpath = QString::fromUtf8(bUrl.c_str(), bUrl.GetLength());
        if (!link.urlpath.contains("http://") && !link.urlpath.contains("https://"))
            link.urlpath.prepend("http://");
    } else {
        const WideString &wFilepath = cAction.GetFilePath();
        link.urlpath = QString::fromWCharArray(wFilepath.c_str(), wFilepath.GetLength());
    }
    return link;
}

DPdfAnnot *DPdfPage::createTextAnnot(QPointF point, QString text)
{
    d_func()->loadPage();

    FPDF_ANNOTATION_SUBTYPE subType = FPDF_ANNOT_TEXT;

    FPDF_ANNOTATION annot = FPDFPage_CreateAnnot(d_func()->m_page, subType);

    if (!FPDFAnnot_SetStringValue(annot, "Contents", text.utf16())) {
        FPDFPage_CloseAnnot(annot);
        return nullptr;
    }

    FS_RECTF rectF;
    rectF.left = static_cast<float>(point.x() - 10);
    rectF.top = static_cast<float>(height() - point.y() + 10);
    rectF.right = static_cast<float>(point.x() + 10);
    rectF.bottom = static_cast<float>(rectF.top - 20);

    if (!FPDFAnnot_SetRect(annot, &rectF)) {
        FPDFPage_CloseAnnot(annot);
        return nullptr;
    }

    FPDFPage_CloseAnnot(annot);

    DPdfTextAnnot *dAnnot = new DPdfTextAnnot;

    dAnnot->setRectF(QRectF(point.x() - 10, point.y() - 10, 20, 20));

    dAnnot->setText(text);

    d_func()->m_dAnnots.append(dAnnot);

    emit annotAdded(dAnnot);

    return dAnnot;
}

bool DPdfPage::updateTextAnnot(DPdfAnnot *dAnnot, QString text, QPointF point)
{
    d_func()->loadPage();

    DPdfTextAnnot *textAnnot = static_cast<DPdfTextAnnot *>(dAnnot);

    if (nullptr == textAnnot)
        return false;

    int index = d_func()->m_dAnnots.indexOf(dAnnot);

    FPDF_ANNOTATION annot = FPDFPage_GetAnnot(d_func()->m_page, index);

    if (!FPDFAnnot_SetStringValue(annot, "Contents", text.utf16())) {
        FPDFPage_CloseAnnot(annot);
        return false;
    }
    textAnnot->setText(text);

    if (!point.isNull()) {
        FS_RECTF rectF;
        rectF.left   = static_cast<float>(point.x() - 10);
        rectF.top    =  static_cast<float>(height() - point.y() + 10);
        rectF.right  = static_cast<float>(point.x() + 10);
        rectF.bottom = static_cast<float>(rectF.top - 20);

        if (!FPDFAnnot_SetRect(annot, &rectF)) {
            FPDFPage_CloseAnnot(annot);
            return false;
        }

        textAnnot->setRectF(QRectF(point.x() - 10, point.y() - 10, 20, 20));
    }

    FPDFPage_CloseAnnot(annot);

    emit annotUpdated(dAnnot);

    return true;
}

DPdfAnnot *DPdfPage::createHightLightAnnot(const QList<QRectF> &list, QString text, QColor color)
{
    d_func()->loadPage();

    FPDF_ANNOTATION_SUBTYPE subType = FPDF_ANNOT_HIGHLIGHT;

    FPDF_ANNOTATION annot = FPDFPage_CreateAnnot(d_func()->m_page, subType);

    if (color.isValid() && !FPDFAnnot_SetColor(annot, FPDFANNOT_COLORTYPE_Color,
                                               static_cast<unsigned int>(color.red()),
                                               static_cast<unsigned int>(color.green()),
                                               static_cast<unsigned int>(color.blue()),
                                               static_cast<unsigned int>(color.alpha()))) {
        FPDFPage_CloseAnnot(annot);
        return nullptr;
    }

    for (const QRectF &rect : list) {
        FS_QUADPOINTSF quad;
        quad.x1 = static_cast<float>(rect.x());
        quad.y1 = static_cast<float>(d_func()->m_height - rect.y());
        quad.x2 = static_cast<float>(rect.x() + rect.width());
        quad.y2 = static_cast<float>(d_func()->m_height - rect.y());
        quad.x3 = static_cast<float>(rect.x());
        quad.y3 = static_cast<float>(d_func()->m_height - rect.y() - rect.height());
        quad.x4 = static_cast<float>(rect.x() + rect.width());
        quad.y4 = static_cast<float>(d_func()->m_height - rect.y() - rect.height());

        if (!FPDFAnnot_AppendAttachmentPoints(annot, &quad))
            continue;
    }

    if (!FPDFAnnot_SetStringValue(annot, "Contents", text.utf16())) {
        FPDFPage_CloseAnnot(annot);
        return nullptr;
    }

    FPDFPage_CloseAnnot(annot);

    DPdfHightLightAnnot *dAnnot = new DPdfHightLightAnnot;

    dAnnot->setBoundaries(list);

    dAnnot->setColor(color);

    dAnnot->setText(text);

    d_func()->m_dAnnots.append(dAnnot);

    emit annotAdded(dAnnot);

    return dAnnot;
}

bool DPdfPage::updateHightLightAnnot(DPdfAnnot *dAnnot, QColor color, QString text)
{
    d_func()->loadPage();

    DPdfHightLightAnnot *hightLightAnnot = static_cast<DPdfHightLightAnnot *>(dAnnot);

    if (nullptr == hightLightAnnot)
        return false;

    int index = d_func()->m_dAnnots.indexOf(dAnnot);

    FPDF_ANNOTATION annot = FPDFPage_GetAnnot(d_func()->m_page, index);

    if (color.isValid()) {
        if (!FPDFAnnot_SetColor(annot, FPDFANNOT_COLORTYPE_Color,
                                static_cast<unsigned int>(color.red()),
                                static_cast<unsigned int>(color.green()),
                                static_cast<unsigned int>(color.blue()),
                                static_cast<unsigned int>(color.alpha()))) {
            FPDFPage_CloseAnnot(annot);
            return false;
        }
        hightLightAnnot->setColor(color);
    }

    if (!FPDFAnnot_SetStringValue(annot, "Contents", text.utf16())) {
        FPDFPage_CloseAnnot(annot);
        return false;
    }
    hightLightAnnot->setText(text);

    FPDFPage_CloseAnnot(annot);

    emit annotUpdated(dAnnot);

    return true;
}

bool DPdfPage::removeAnnot(DPdfAnnot *dAnnot)
{
    d_func()->loadPage();

    int index = d_func()->m_dAnnots.indexOf(dAnnot);

    if (index < 0)
        return false;

    if (!FPDFPage_RemoveAnnot(d_func()->m_page, index))
        return false;

    d_func()->m_dAnnots.removeAll(dAnnot);

    emit annotRemoved(dAnnot);

    delete dAnnot;

    return true;
}

QVector<QRectF> DPdfPage::search(const QString &text, bool matchCase, bool wholeWords)
{
    d_func()->loadTextPage();

    QVector<QRectF> rectfs;
    unsigned long flags = 0x00000000;

    if (matchCase)
        flags |= FPDF_MATCHCASE;

    if (wholeWords)
        flags |= FPDF_MATCHWHOLEWORD;

    FPDF_SCHHANDLE schandle = FPDFText_FindStart(d_func()->m_textPage, text.utf16(), flags, 0);
    if (schandle) {
        while (FPDFText_FindNext(schandle)) {
            int curSchIndex = FPDFText_GetSchResultIndex(schandle);
            if (curSchIndex >= 0) {
                const QVector<QRectF> &textrectfs = getTextRects(curSchIndex, text.length());
                rectfs << textrectfs;
            }
        };
    }

    FPDFText_FindClose(schandle);
    return rectfs;
}

QList<DPdfAnnot *> DPdfPage::annots()
{
    d_func()->loadAnnots();

    QList<DPdfAnnot *> annots;

    foreach (DPdfAnnot *annot, d_func()->m_dAnnots) {
        if (annot->type() == DPdfAnnot::AText || annot->type() == DPdfAnnot::AHighlight) {
            annots.append(annot);
            continue;
        }
    }

    return annots;
}

QList<DPdfAnnot *> DPdfPage::links()
{
    d_func()->loadAnnots();

    QList<DPdfAnnot *> links;

    foreach (DPdfAnnot *annot, d_func()->m_dAnnots) {
        if (annot->type() == DPdfAnnot::ALink) {
            links.append(annot);
            continue;
        }
    }

    return links;
}
