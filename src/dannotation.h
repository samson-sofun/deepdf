#ifndef DANNOTATION_H
#define DANNOTATION_H

#include "dpdfiumglobal.h"

#include <QRectF>

class DEEPIN_PDFIUM_EXPORT DAnnotation
{
public:
    enum AnnotationType {
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

private:
    friend class DPdfiumPagePrivate;
    DAnnotation(int type);

    int m_type;
    QRectF m_boundary;
};


#endif // DANNOTATION_H
