# deepinpdf

deepinpdf是一个基于Qt和PDFium的PDF加载库，对PDFium最新版本进行裁剪内置并使用了Qt风格的语法对其进行封装。

### 环境
已在多个cpu架构下验证通过如x86/x64/mips/arm，可跨系统平台但目前只做了linux debian版本。

### 安装
如果想作为库形式使用就用以下方式安装到系统目录:

```sh
git clone <url>
cd deepin-pdfium
qmake
make
make install (might need sudo)
```

### 使用
拷贝dpidfium.h和相关头文件到你的工程里，并链接生成的libdpdf.so

使用方法很简单，如打开一个名为1.pdf的文档并且获取第一页图片

```sh
DPdfium *document = new DPdfium("1.pdf");
DPdfiumPage *page = document->page(0);
QImage image = page->image();
```

### 贡献
欢迎fork并发起合并请求

### License
LGPL-3.0
