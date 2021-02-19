# deepdf

deepdf is a pdf rendering library based on Qt and PDFium, which can cross system platforms.

It has been verified under several CPU architectures, such as x86 / x64 / MIPS / arm.

### Installation
You can install it into the system for easy use. Here is an example on Linux/Mac:

```sh
git clone <url>
cd deepdf
qmake
make
sudo make install
```

### Usage
Copy the public header files into your project and link so files like:

```sh
LIBS += -lopenjp2 -llcms2 -lfreetype -ldeepdf
```

It's easy to use, for example, to open a document called 1.PDF and get the first page image

```sh
DPdfDoc *doc = new DPdfdoc("1.pdf");
DPdfPage *page = doc->page(0);
QImage image = page->image();
```

### License
LGPL-3.0

