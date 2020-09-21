#ifndef DPDFGLOBAL_H
#define DPDFGLOBAL_H

#include <QtCore/qglobal.h>

class DPDFGlobal
{
public:
    DPDFGlobal();
    ~DPDFGlobal();

private:
    void initPdfium();
    void shutdownPdfium();
};

#endif // DPDFGLOBAL_H
