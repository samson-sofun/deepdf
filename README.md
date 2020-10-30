# deepin-pdfium

deepin-pdfium is a pdf rendering library based on Qt and PDFium.

It has been verified under several CPU architectures, such as x86 / x64 / MIPS / arm, which can cross system platforms, but only Linux Debian version has been made.

### Installation
To use the library you must first install it in the same location as your Qt installation. Here is an example on Linux/Mac:

```sh
git clone <url>
cd deepin-pdfium
qmake
make
sudo make install
```

### Usage
Copy the public header files into your project and link libdpdf.so

It's easy to use, for example, to open a document called 1.PDF and get the first page image

```sh
DPdfium *document = new DPdfium("1.pdf");
DPdfiumPage *page = document->page(0);
QImage image = page->image();
```

### License
LGPL-3.0

