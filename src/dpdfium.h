#ifndef DPdfium_H
#define DPdfium_H

#include <QObject>
#include <QMap>
#include <QWeakPointer>
#include <QSharedPointer>
#include <QVector>

#include "dpdfiumpage.h"

class FPDF_Document;
class PageHolder;

class DEEPIN_PDFIUM_EXPORT DPdfium
{
public:
    enum Status {
        NOT_LOADED = -1,
        SUCCESS = 0,
        FILE_ERROR,
        FORMAT_ERROR,
        PASSWORD_ERROR,
        HANDLER_ERROR,
        FILE_NOT_FOUND_ERROR
    };

    struct Section;
    typedef QVector< Section > Outline;
    struct Section {
        int nIndex;
        QPointF offsetPointF;
        QString title;
        Outline children;
    };

    explicit DPdfium();
    DPdfium(QString filename, QString password = QString());

    virtual ~DPdfium();

    /**
     * @brief isValid
     * 文档是否有效
     * @return
     */
    bool isValid() const;

    /**
     * @brief isEncrypted
     * 是否是加密文档
     * @return
     */
    bool isEncrypted() const;

    /**
     * @brief filename
     * 文档路径
     * @return
     */
    QString filename() const;

    /**
     * @brief pageCount
     * 文档页数
     * @return
     */
    int pageCount() const;

    /**
     * @brief status
     * 文档状态
     * @return
     */
    Status status() const;

    /**
     * @brief page
     * 返回指定PAGE
     * @param i
     * @return
     */
    DPdfiumPage *page(int i);

    /**
     * @brief outline
     * 目录
     * @return
     */
    Outline outline();

public Q_SLOTS:
    /**
     * @brief loadFile
     * 加载文档
     * @param filename
     * @param password
     * @return
     */
    Status loadFile(QString filename, QString password = QString());

private:
    Q_DISABLE_COPY(DPdfium)

    QSharedPointer<FPDF_Document> m_document;
    QVector<QWeakPointer<PageHolder>> m_pages;
    QString m_filename;
    int m_pageCount;
    Status m_status;
    DPdfium::Status parseError(int err);
};

#endif // DPdfium_H
