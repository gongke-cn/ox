#!/bin/bash

TOP=`pwd`

function build() {
    echo build $ARCH || exit 1
    if [ "x$CC" = "x" ]; then
        CC=gcc
    fi
    make O=o-$ARCH $@ basic -j32 || exit 1
    make O=o-$ARCH $@ env -j32 Q= || exit 1
    make O=o-$ARCH $@ PB_LIBS=-L$TOP/o-$ARCH/$LIB_ARCH Q= packages || exit 1
    mkdir -p release/$ARCH/oxp
    cp o-$ARCH/oxp/*.oxp* o-$ARCH/oxp/package_list.ox release/$ARCH/oxp
    if [ "$ARCH" != "x86_64-w64-windows-gnu" ]; then
        make O=o-$ARCH deb $@
        mkdir -p release/deb
        cp o-$ARCH/*.deb release/deb
    fi
}

if [ "$1" = "clean" ]; then
    rm -rf o-x86_64-pc-linux-gnu o-i686-pc-linux-gnu o-x86_64-w64-windows-gnu
elif [ "$1" = "x86_64-pc-linux-gnu" ]; then
    ARCH=x86_64-pc-linux-gnu build
elif [ "$1" = "i686-pc-linux-gnu" ]; then
    ARCH=i686-pc-linux-gnu CC='i686-linux-gnu-gcc' build CROSS_PREFIX=i686-linux-gnu- PKGCONFIG_PREFIX=i386-linux-gnu- INTERNAL_PKGS=1
elif [ "$1" = "x86_64-w64-windows-gnu" ]; then
    ARCH=x86_64-w64-windows-gnu build CROSS_PREFIX=x86_64-w64-mingw32- PKGCONFIG_PREFIX=x86_64-w64-mingw32-
else
    ARCH=x86_64-pc-linux-gnu build M=64
    ARCH=i686-pc-linux-gnu CC='i686-linux-gnu-gcc' build CROSS_PREFIX=i686-linux-gnu- PKGCONFIG_PREFIX=i386-linux-gnu- INTERNAL_PKGS=1
    #ARCH=x86_64-w64-windows-gnu build CROSS_PREFIX=x86_64-w64-mingw32- PKGCONFIG_PREFIX=x86_64-w64-mingw32-
fi
