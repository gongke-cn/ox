# 安装OX环境
## 符号说明
注意以下文档中使用了一些特定符号，其定义如下：

* %VERSION%: 用户使用OX语言环境的版本号。

## Ubuntu Linux
根据安装平台下载OX语言对应的deb安装包。如在"x86"64位系统下下载安装包"ox_%VERSION%_amd64.deb"，在"x86"32位系统下下载安装包"ox_%VERSION%_i386.deb"。运行以下命令安装：
```
sudo dpkg --install ox_%VERSION%_amd64.deb
```
## Fedora Linux
根据安装平台下载OX语言对应的rpm安装包。运行以下命令安装：
```
rpm -i ox-%VERSION%-1.fc42.x86_64.rpm
```
## Windows
下载windows安装程序"ox-windows-%VERSION%-installer.exe"。以管理员方式运行"ox-windows-%VERSION%-installer.exe"进行安装。
## 从源码构建
注意编译OX需要提前安装以下软件：

* gcc: OX采用gcc和GNU工具链进行编译。
* GNU make：OX采用GNU make编写构建脚本。
* clang：OX可以用clang代替gcc进行编译。同时OX可以使用clang解析C语言头文件，创建OX原生模块的源代码。
* [pkgconf](http://pkgconf.org/): OX利用pkgconf检测依赖库的编译参数。
* gettext：OX采用GNU gettext工具进行多语言和本地化处理。
* [libffi](https://sourceware.org/libffi/)
* [libcurl](https://curl.se/): 包管理器依赖
* [libarchive](https://www.libarchive.org/): 包管理器依赖
* [openssl](https://www.openssl.org/) 如果需要生成"ssl"软件包需要此库。
* [ncurses](https://invisible-island.net/ncurses/) 如果需要生成"ncurses"软件包需要此库。
* [SDL](https://www.libsdl.org/) 如果需要生成"sdl"软件包需要此库。
* [glib2](https://docs.gtk.org/glib/) 如果需要生成"gir"软件包需要此库。

如果在windows下编译，需安装[msys2环境](https://www.msys2.org/)并在msys2 ucrt64环境下进行编译。

从git仓库同步最新代码：
```
git clone https://gitee.com/gongke1978/ox.git
```
OX采用GNU make编写构建脚本，Makefile中可以指定以下参数：

* O: 指定编译过程中生成临时文件的输出目录。缺省输出目录为"out"。
* Q: 缺省make采用安静模式，如果想看到构建过程中执行的命令，指定"Q="。
* INSTALL_PREFIX: 指定安装目录，缺省安装目录为"/usr"。如运行"make INSTALL_PREFIX=/usr/local install-basic"则将程序安装到"/usr/local/bin"目录下。在windows下如运行"make INSTALL_PREFIX=C:/msys64/ucrt64"则将程序安装到C盘“/msys64/ucrt64”目录下。
* DEBUG: 缺省编译时不带调试信息，如果希望带调试信息，指定"DEBUG=1"。
* CROSS_PREFIX: 如果需要进行交叉编译，指定交叉编译工具链的前缀，如需要使用"arm-linux-gnu-gcc"进行编译，则指定参数"CROSS_PREFIX=arm-linux-gnu-"。
* PKGCONFIG_PREFIX: 如果需要进行交叉编译，指定"pkg-config"程序的前缀，如需要使用"arm-linux-gnu-pkg-config"进行编译，则指定参数"PKGCONFIG_PREFIX=arm-linux-gnu-"。
* CLANG: 缺省OX使用gcc进行编译，如果想用clang进行编译，指定"CLANG=1"。


编译基本OX库和可执行程序：
```
make basic
```
将OX库和可执行程序安装到系统中：
```
make install-basic
```
编译OX基本环境所需的包：
```
make env
```
安装OX基本环境到系统中：
```
make install-env
```
编译OX软件包:
```
make packages
```
安装OX软件包到系统中:
```
make install-packages
```