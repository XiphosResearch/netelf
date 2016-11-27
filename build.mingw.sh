#!/bin/sh
echo Small.64
x86_64-w64-mingw32-gcc -Os -s -DNODEFAULTLIB -o bin/netelf.Win32.Mingw64.small.exe netelf.c -nostdlib -lkernel32 -lwsock32 -mwindows
ls -lah bin/netelf.Win32.Mingw64.small.exe
echo Normal.64
x86_64-w64-mingw32-gcc -o bin/netelf.Win32.Mingw64.normal.exe netelf.c -lwsock32 -mconsole 
ls -lah bin/netelf.Win32.Mingw64.normal.exe

echo Small.32
i686-w64-mingw32-gcc -Os -s -DNODEFAULTLIB -o bin/netelf.Win32.Mingw32.small.exe netelf.c -nostdlib -lkernel32 -lwsock32 -mwindows
ls -lah bin/netelf.Win32.Mingw32.small.exe
echo Normal.32
i686-w64-mingw32-gcc -o bin/netelf.Win32.Mingw32.normal.exe netelf.c -lwsock32 -mconsole 
ls -lah bin/netelf.Win32.Mingw32.normal.exe
