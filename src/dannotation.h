#ifndef DANNOTATION_H
#define DANNOTATION_H

#include "dpdfiumglobal.h"

#include <QRectF>

class DEEPIN_PDFIUM_EXPORT DAnnotation
{
public:
    DAnnotation();

    enum AnnotationType
    {
        IconAnnotation = 1,            ///< IconAnnotation
        HighlightAnnotation = 2,       ///< HighlightAnnotation
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
    void setBoundary( const QRectF &boundary );

};

/**
 * Showing some text using an icon shown on a page.
 */
class DEEPIN_PDFIUM_EXPORT DIconAnnotation : public DAnnotation
{
public:
    DIconAnnotation();
};

#endif // DANNOTATION_H
