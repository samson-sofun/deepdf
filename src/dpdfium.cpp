#include "dpdfium.h"
#include "dpdfiumpage.h"

#include "public/fpdfview.h"
#include "public/fpdf_doc.h"

#include "core/fpdfdoc/cpdf_bookmark.h"
#include "core/fpdfdoc/cpdf_bookmarktree.h"
#include "core/fpdfapi/parser/cpdf_document.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"

#include <QFile>
#include <QDebug>

DPdfium::DPdfium()
    : m_documentHandler(nullptr)
    , m_pageCount(0)
    , m_status(NOT_LOADED)
{

}

DPdfium::DPdfium(QString filename, QString password)
    : m_documentHandler(nullptr)
    , m_pageCount(0)
    , m_status(NOT_LOADED)
{
    loadFile(filename, password);
}

DPdfium::~DPdfium()
{
    m_pages.clear();

    if (nullptr != m_documentHandler)
        FPDF_CloseDocument((FPDF_DOCUMENT)m_documentHandler);
}

bool DPdfium::isValid() const
{
    return m_documentHandler != NULL;
}

bool DPdfium::isEncrypted() const
{
    if (!isValid())
        return false;

    return FPDF_GetDocPermissions((FPDF_DOCUMENT)m_documentHandler) != 0xFFFFFFFF;
}

DPdfium::Status DPdfium::loadFile(QString filename, QString password)
{
    m_filename = filename;

    m_pages.clear();

    if (!QFile::exists(filename)) {
        m_status = FILE_NOT_FOUND_ERROR;
        return m_status;
    }

    void *ptr = FPDF_LoadDocument(m_filename.toUtf8().constData(),
                                  password.toUtf8().constData());

    m_documentHandler = static_cast<DPdfiumDocumentHandler *>(ptr);

    m_status = m_documentHandler ? SUCCESS : parseError(FPDF_GetLastError());

    if (m_documentHandler) {
        m_pageCount = FPDF_GetPageCount((FPDF_DOCUMENT)m_documentHandler);
        m_pages.fill(nullptr, m_pageCount);
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

    if (!m_pages[i])
        m_pages[i] = new DPdfiumPage(m_documentHandler, i);

    return m_pages[i];
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
    CPDF_BookmarkTree tree(reinterpret_cast<CPDF_Document *>(m_documentHandler));
    CPDF_Bookmark cBookmark;
    const CPDF_Bookmark &firstRootChild = tree.GetFirstChild(&cBookmark);
    if (firstRootChild.GetDict() != NULL)
        collectBookmarks(outline, tree, firstRootChild);

    return outline;
}

DPdfium::Properies DPdfium::proeries()
{
    Properies properies;
    int fileversion = 1;
    properies.insert("Version", "1");
    if (FPDF_GetFileVersion((FPDF_DOCUMENT)m_documentHandler, &fileversion)) {
        properies.insert("Version", fileversion);
    }
    properies.insert("Encrypted", isEncrypted());
    properies.insert("Linearized", FPDF_GetFileLinearized((FPDF_DOCUMENT)m_documentHandler));

    properies.insert("KeyWords", QString());
    properies.insert("Title", QString());
    properies.insert("Creator", QString());
    properies.insert("Producer", QString());
    CPDF_Document *pDoc = reinterpret_cast<CPDF_Document *>(m_documentHandler);
    const CPDF_Dictionary *pInfo = pDoc->GetInfo();
    if (pInfo) {
        const WideString &KeyWords = pInfo->GetUnicodeTextFor("Keywords");
        properies.insert("KeyWords", QString::fromWCharArray(KeyWords.c_str(), KeyWords.GetLength()));

        const WideString &Title = pInfo->GetUnicodeTextFor("Title");
        properies.insert("Title", QString::fromWCharArray(Title.c_str(), Title.GetLength()));

        const WideString &Creator = pInfo->GetUnicodeTextFor("Creator");
        properies.insert("Creator", QString::fromWCharArray(Creator.c_str(), Creator.GetLength()));

        const WideString &Producer = pInfo->GetUnicodeTextFor("Producer");
        properies.insert("Producer", QString::fromWCharArray(Producer.c_str(), Producer.GetLength()));
    }

    qDebug() << "properies = " << properies;
    return properies;
}

