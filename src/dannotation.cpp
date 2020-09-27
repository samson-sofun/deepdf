#include "dannotation.h"

#include "public/fpdf_annot.h"

DAnnotation::DAnnotation(int type)
{
    m_type = type;
}

QRectF DAnnotation::boundary() const
{
    return m_boundary;
}

void DAnnotation::setBoundary(const QRectF &boundary)
{
    m_boundary = boundary;
}
