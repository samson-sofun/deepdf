#include "dpdfannot.h"

#include "public/fpdf_annot.h"

DPdfAnnot::DPdfAnnot(AnnotType type)
{
    m_type = type;
}

QRectF DPdfAnnot::boundary() const
{
    return m_boundary;
}

void DPdfAnnot::setBoundary(const QRectF &boundary)
{
    m_boundary = boundary;
}

DPdfAnnot::AnnotType DPdfAnnot::type()
{
    return m_type;
}

void DPdfAnnot::setType(DPdfAnnot::AnnotType type)
{
    m_type = type;
}
