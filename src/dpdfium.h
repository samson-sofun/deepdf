#ifndef DPdfium_H
#define DPdfium_H

#include "dpdfiumglobal.h"

#include <QObject>
#include <QMap>
#include <QVector>
#include <QPointF>

class DPdfiumPage;
class DPdfiumDocumentHandler;
class DEEPIN_PDFIUM_EXPORT DPdfium : public QObject
{
    Q_OBJECT
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
    typedef QMap<QString, QVariant> Properies;

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
     * @brief 文档是否有效
     * @return
     */
    bool isValid() const;

    /**
     * @brief 是否是加密文档
     * @return
     */
    bool isEncrypted() const;

    /**
     * @brief 文档路径
     * @return
     */
    QString filename() const;

    /**
     * @brief 文档页数
     * @return
     */
    int pageCount() const;

    /**
     * @brief 文档状态
     * @return
     */
    Status status() const;

    /**
     * @brief 返回指定PAGE
     * @param i
     * @return
     */
    DPdfiumPage *page(int i);

    /**
     * @brief 目录
     * @return
     */
    Outline outline();

    /**
     * @brief 文档属性信息
     * @return
     */
    Properies proeries();

public Q_SLOTS:
    /**
     * @brief 加载文档
     * @param filename
     * @param password
     * @return
     */
    Status loadFile(QString filename, QString password = QString());

private:
    Q_DISABLE_COPY(DPdfium)

    DPdfiumDocumentHandler *m_documentHandler = nullptr;
    QVector<DPdfiumPage *> m_pages;
    QString m_filename;
    int m_pageCount;
    Status m_status;
    DPdfium::Status parseError(int err);
};

#endif // DPdfium_H
