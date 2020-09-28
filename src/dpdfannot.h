#ifndef DPDFANNOT_H
#define DPDFANNOT_H

#include "dpdfglobal.h"

#include <QRectF>

class DEEPIN_PDFIUM_EXPORT DPdfAnnot
{
public:
    enum AnnotType {
        AUnknown = 0,         ///< 前期支持以外的
        AText = 1,            ///< TextAnnotation
        AHighlight = 2,       ///< HighlightAnnotation
        ALink = 3
    };

    /**
     * @brief  Returns this annotation's boundary rectangle(width/height is 1 base)
     * @return
     */
    QRectF boundary() const;

    /**
     * @brief Sets this annotation's boundary rectangle
     * @param boundary
     */
    void setBoundary(const QRectF &boundary);

    AnnotType type();

    void setType(AnnotType type);

private:
    friend class DPdfPagePrivate;

    DPdfAnnot(AnnotType type);

    AnnotType m_type;

    QRectF m_boundary;
};


#endif // DPDFANNOT_H
