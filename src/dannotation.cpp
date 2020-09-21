#include "dannotation.h"

DAnnotation::DAnnotation()
{

}

QRectF DAnnotation::boundary() const
{
    return QRectF();
}

void DAnnotation::setBoundary(const QRectF &boundary)
{

}

DIconAnnotation::DIconAnnotation():DAnnotation()
{

}
