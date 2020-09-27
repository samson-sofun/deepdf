#ifndef DPdfiumPAGE_H
#define DPdfiumPAGE_H

#include <QObject>
#include <QImage>
#include <QSharedPointer>

#include "dpdfiumglobal.h"

class DPdfium;
class DAnnotation;
class DPdfiumPagePrivate;
class DPdfiumDocumentHandler;
class DEEPIN_PDFIUM_EXPORT DPdfiumPage
{
public:
    ~DPdfiumPage();

    /**
     * @brief width
     * 图片宽
     * @return
     */
    qreal width() const;

    /**
     * @brief height
     * 图片高
     * @return
     */
    qreal height() const;

    /**
     * @brief isValid
     * 当页是否有效
     * @return
     */
    bool isValid() const;

    /**
     * @brief pageIndex
     * 当页索引
     * @return
     */
    int pageIndex() const;

    /**
     * @brief 获取范围内图片
     * @param scale
     * @return
     */
    QImage image(qreal xscale = 1, qreal yscale = 1, qreal x = 0, qreal y = 0, qreal width = 0, qreal height = 0);

    /**
     * @brief countChars
     * 字符数
     * @return
     */
    int countChars() const;

    /**
     * @brief getTextRects
     * 根据索引获取文本范围
     * @param start
     * @param charCount
     * @return
     */
    QVector<QRectF> getTextRects(int start = 0, int charCount = -1) const;

    /**
     * @brief text
     * 根据范围获取文本
     * @param rect
     * @return
     */
    QString text(const QRectF &rect) const;

    /**
     * @brief text
     * 获取总文本
     * @return
     */
    QString text() const;

    /**
     * @brief text
     * 根据索引获取文本
     * @param start
     * @param charCount
     * @return
     */
    QString text(int start, int charCount) const;

    /**
     * @brief label
     * 下标真实页码
     * @return
     */
    QString label() const;

    QList<DAnnotation*> annotations();

    bool deleteAnnotation(int index);

private:
    DPdfiumPage(DPdfiumDocumentHandler *handler, int pageIndex);

    QSharedPointer<DPdfiumPagePrivate> m_private;

    int m_index;

    friend class DPdfium;
};

#endif // DPdfiumPAGE_H
