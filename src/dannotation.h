#ifndef DANNOTATION_H
#define DANNOTATION_H

#include "dpdfiumglobal.h"

#include <QRectF>

class DEEPIN_PDFIUM_EXPORT DAnnotation
{
public:
    DAnnotation();

    enum AnnotationType {
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
    int m_type;
};


#endif // DANNOTATION_H
