#!/bin/bash

if [ "x$1" = "xclean" ]; then
	make dist-clean
else
    make basic -j32
    sudo make install-basic
    make env -j32
    sudo make install-env
    make packages
    sudo make install-packages
fi
