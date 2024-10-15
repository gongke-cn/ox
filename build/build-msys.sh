#!/bin/bash

if [ "x$1" = "xclean" ]; then
	make O=o-msys dist-clean
else
	make O=o-msys INSTALL_PREFIX=C:/msys64/ucrt64 DEBUG=1 install-basic -j4 || exit 1
	make O=o-msys INSTALL_PREFIX=C:/msys64/ucrt64 DEBUG=1 install-env -j4 || exit 1
	make O=o-msys INSTALL_PREFIX=C:/msys64/ucrt64 DEBUG=1 install-packages || exit 1
	mkdir -p release/x86_64-w64-windows-gnu/oxp
	cp o-msys/oxp/*.oxp* o-msys/oxp/package_list.ox release/x86_64-w64-windows-gnu/oxp
	make O=o-msys INSTALL_PREFIX=C:/msys64/ucrt64 wininst || exit 1
	mkdir -p release/windows
	cp o-msys/ox-windows-*-installer.exe release/windows
fi
