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
class DEEPIN_PDFIUM_EXPORT DPdfiumPage : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(DPdfiumPage)
    
public:
    ~DPdfiumPage();

    /**
     * @brief 图片宽
     * @return
     */
    qreal width() const;

    /**
     * @brief 图片高
     * @return
     */
    qreal height() const;

    /**
     * @brief 当页索引
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
     * @brief 字符数
     * @return
     */
    int countChars() const;

    /**
     * @brief 根据索引获取文本范围
     * @param start
     * @param charCount
     * @return
     */
    QVector<QRectF> getTextRects(int start = 0, int charCount = -1) const;

    /**
     * @brief 根据范围获取文本
     * @param rect
     * @return
     */
    QString text(const QRectF &rect) const;

    /**
     * @brief 获取总文本
     * @return
     */
    QString text() const;

    /**
     * @brief 根据索引获取文本
     * @param start
     * @param charCount
     * @return
     */
    QString text(int start, int charCount) const;

    /**
     * @brief 下标真实页码
     * @return
     */
    QString label() const;

    /**
     * @brief 获取当前所有注释
     * @return
     */
    QList<DAnnotation *> annotations();

    /**
     * @brief 删除注释
     * @param annot 即将删除的注释指针，执行成功传入的指针会被删除
     * @return
     */
    bool removeAnnotation(DAnnotation *annot);

signals:
    /**
     * @brief 添加注释时触发 ，在需要的时候可以重新获取annotations()
     * @param 增加后的index
     */
    void annotationAdded(int index);

    /**
     * @brief 注释被删除时触发 ，在需要的时候可以重新获取annotations()
     * @param index 被移除的index
     */
    void annotationRemoved(int index);

private:
    DPdfiumPage(DPdfiumDocumentHandler *handler, int pageIndex);

    QScopedPointer<DPdfiumPagePrivate> d_ptr;

    int m_index;

    friend class DPdfium;
};

#endif // DPdfiumPAGE_H
