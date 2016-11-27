#!/bin/sh
OS=`uname -s`
LDLIBS=
if [ "$OS" = "SunOS" ]; then
	LDLIBS="-lnsl -lsocket"
fi
cc -o bin/netelf.$OS.`uname -m`.exe netelf.c $LDLIBS
cc -o bin/test.$OS.`uname -m`.exe test.c
