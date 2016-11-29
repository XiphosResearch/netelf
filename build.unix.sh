#!/bin/sh
OS=`uname -s`
LDLIBS=
if [ "$OS" = "SunOS" ]; then
	LDLIBS="-lnsl -lsocket"
elif [ "$OS" = "Linux" ]; then
	LDLIBS="-lrt"
fi
cc -o bin/netelf.$OS.`uname -m`.exe netelf.c $LDLIBS
cc -o bin/test.$OS.`uname -m`.exe test.c
cc -o bin/test_loaddll.$OS.`uname -m`.exe test_loaddll.c -ldl
