# Install OX Environment
## Symbol Explanation
Note that some specific symbols are used in the following document, and their definitions are as follows:

* %VERSION%: The version number of the OX language environment used by the user.

## Ubuntu Linux
Download the corresponding deb installation package of the OX language according to the installation platform. For example, download the installation package "ox_%VERSION%_amd64.deb" for the "x86" 64-bit system, and download the installation package "ox_%VERSION%_i386.deb" for the "x86" 32-bit system. Run the following command to install:
```
sudo dpkg --install ox_%VERSION%_amd64.deb
```
## Fedora Linux
Download the corresponding rpm installation package of the OX language according to the installation platform. Run the following command to install:
```
rpm -i ox-%VERSION%-1.fc42.x86_64.rpm
```
## Windows
Download the Windows installer "ox-windows-%VERSION%-installer.exe". Run "ox-windows-%VERSION%-installer.exe" as an administrator to install.
## Build from Source Code
Note that compiling OX requires the following software to be installed in advance:

* gcc: OX is compiled with gcc and the GNU toolchain.
* GNU make: OX uses GNU make to write build scripts.
* clang: OX can be compiled with clang instead of gcc. Meanwhile, OX can use clang to parse C language header files and create source code for OX native modules.
* [pkgconf](http://pkgconf.org/): OX uses pkgconf to detect the compilation parameters of dependent libraries.
* gettext: OX uses the GNU gettext tool for multilingual and localization processing.
* [libffi](https://sourceware.org/libffi/)
* [libcurl](https://curl.se/): Required by the package manager
* [libarchive](https://www.libarchive.org/): Required by the package manager
* [openssl](https://www.openssl.org/) This library is required if you need to generate the "ssl" package.
* [ncurses](https://invisible-island.net/ncurses/) This library is required if you need to generate the "ncurses" package.
* [SDL](https://www.libsdl.org/) This library is required if you need to generate the "sdl" package.
* [glib2](https://docs.gtk.org/glib/) This library is required if you need to generate the "gir" package.

If compiling on Windows, you need to install the [msys2 environment](https://www.msys2.org/) and compile in the msys2 ucrt64 environment.

Synchronize the latest code from the git repository:
```
git clone https://gitee.com/gongke1978/ox.git
```
OX uses GNU make to write build scripts, and the following parameters can be specified in the Makefile:

* O: Specify the output directory for temporary files generated during compilation. The default output directory is "out".
* Q: By default, make uses quiet mode. If you want to see the commands executed during the build process, specify "Q=".
* INSTALL_PREFIX: Specify the installation directory, the default installation directory is "/usr". For example, running "make INSTALL_PREFIX=/usr/local install-basic" will install the program to the "/usr/local/bin" directory. On Windows, running "make INSTALL_PREFIX=C:/msys64/ucrt64" will install the program to the "C:/msys64/ucrt64" directory on the C drive.
* DEBUG: By default, no debugging information is included during compilation. If you want to include debugging information, specify "DEBUG=1".
* CROSS_PREFIX: If cross-compilation is required, specify the prefix of the cross-compilation toolchain. For example, to compile with "arm-linux-gnu-gcc", specify the parameter "CROSS_PREFIX=arm-linux-gnu-".
* PKGCONFIG_PREFIX: If cross-compilation is required, specify the prefix of the "pkg-config" program. For example, to compile with "arm-linux-gnu-pkg-config", specify the parameter "PKGCONFIG_PREFIX=arm-linux-gnu-".
* CLANG: By default, OX is compiled with gcc. If you want to compile with clang, specify "CLANG=1".


Compile the basic OX library and executable program:
```
make basic
```
Install the OX library and executable program into the system:
```
make install-basic
```
Compile the packages required for the basic OX environment:
```
make env
```
Install the basic OX environment into the system:
```
make install-env
```
Compile OX software packages:
```
make packages
```
Install OX software packages into the system:
```
make install-packages
```