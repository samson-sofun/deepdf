#include "dpdfium.h"
#include "public/fpdfview.h"

#include "core/fpdfdoc/cpdf_bookmark.h"
#include "core/fpdfdoc/cpdf_bookmarktree.h"

#include <QFile>
#include <QDebug>

QT_BEGIN_NAMESPACE

DPdfium::DPdfium()
    : m_document(nullptr)
    , m_pageCount(0)
    , m_status(NOT_LOADED)
{
}

DPdfium::DPdfium(QString filename, QString password)
    : m_document(nullptr)
    , m_pageCount(0)
    , m_status(NOT_LOADED)
{
    loadFile(filename, password);
}

DPdfium::~DPdfium()
{
    m_pages.clear();
    m_document.clear();
}

bool DPdfium::isValid() const
{
    return m_document != NULL;
}

DPdfium::Status DPdfium::loadFile(QString filename, QString password)
{
    m_filename = filename;

    m_pages.clear();
    m_document.clear();

    if (!QFile::exists(filename)) {
        m_status = FILE_NOT_FOUND_ERROR;
        return m_status;
    }

    void *ptr = FPDF_LoadDocument(m_filename.toUtf8().constData(),
                                  password.toUtf8().constData());

    auto doc = static_cast<FPDF_Document *>(ptr);
    m_document.reset(doc);
    m_status = m_document ? SUCCESS : parseError(FPDF_GetLastError());


    if (m_document) {
        m_pageCount = FPDF_GetPageCount((FPDF_DOCUMENT)m_document.data());
        m_pages.resize(m_pageCount);
    }

    return m_status;
}

DPdfium::Status DPdfium::parseError(int err)
{
    DPdfium::Status err_code = DPdfium::SUCCESS;
    // Translate FPDFAPI error code to FPDFVIEW error code
    switch (err) {
    case FPDF_ERR_SUCCESS:
        err_code = DPdfium::SUCCESS;
        break;
    case FPDF_ERR_FILE:
        err_code = DPdfium::FILE_ERROR;
        break;
    case FPDF_ERR_FORMAT:
        err_code = DPdfium::FORMAT_ERROR;
        break;
    case FPDF_ERR_PASSWORD:
        err_code = DPdfium::PASSWORD_ERROR;
        break;
    case FPDF_ERR_SECURITY:
        err_code = DPdfium::HANDLER_ERROR;
        break;
    }
    return err_code;
}

QString DPdfium::filename() const
{
    return m_filename;
}

int DPdfium::pageCount() const
{
    return m_pageCount;
}

DPdfium::Status DPdfium::status() const
{
    return m_status;
}

DPdfiumPage *DPdfium::page(int i)
{
    if (i < 0 || i >= m_pageCount)
        return nullptr;

    auto strongRef = m_pages[i].toStrongRef();
    if (!strongRef)
        strongRef.reset(new PageHolder(m_document.toWeakRef(),
                                       reinterpret_cast<FPDF_Page *>(FPDF_LoadPage((FPDF_DOCUMENT)m_document.data(), i))));

    m_pages[i] = strongRef.toWeakRef();
    return new DPdfiumPage(strongRef, i);
}

void collectBookmarks(DPdfium::Outline &outline, const CPDF_BookmarkTree &tree, CPDF_Bookmark This)
{
    DPdfium::Section section;

    const WideString &title = This.GetTitle();
    section.title = QString::fromWCharArray(title.c_str(), title.GetLength());

    CPDF_Dest &&dest = This.GetDest(tree.GetDocument());
    section.nIndex = dest.GetDestPageIndex(tree.GetDocument());

    bool hasx = false, hasy = false, haszoom = false;
    float x = 0.0, y = 0.0, z = 0.0;
    dest.GetXYZ(&hasx, &hasy, &haszoom, &x, &y, &z);
    section.offsetPointF = QPointF(x, y);

    const CPDF_Bookmark &Child = tree.GetFirstChild(&This);
    if (Child.GetDict() != NULL) {
        collectBookmarks(section.children, tree, Child);
    }
    outline << section;
    qDebug() << "outline  = " << section.title << section.nIndex << section.offsetPointF;

    const CPDF_Bookmark &SibChild = tree.GetNextSibling(&This);
    if (SibChild.GetDict() != NULL) {
        collectBookmarks(outline, tree, SibChild);
    }
}

DPdfium::Outline DPdfium::outline()
{
    Outline outline;
    CPDF_BookmarkTree tree(reinterpret_cast<CPDF_Document *>(m_document.data()));
    CPDF_Bookmark cBookmark;
    const CPDF_Bookmark &firstRootChild = tree.GetFirstChild(&cBookmark);
    if (firstRootChild.GetDict() != NULL)
        collectBookmarks(outline, tree, firstRootChild);

    return outline;
}

QT_END_NAMESPACE
