#!/bin/sh
SMALLC_FLAGS="-nostdlib -O2 -s -DNODEFAULTLIB -mwindows -fmerge-all-constants -fno-stack-protector -fno-ident -fomit-frame-pointer -fno-unwind-tables -fno-asynchronous-unwind-tables -ffunction-sections -fdata-sections -flto -Wl,--gc-sections -Wl,--build-id=none"

echo Small.64
x86_64-w64-mingw32-gcc $SMALLC_FLAGS -o bin/netelf.Win32.Mingw64.small.exe netelf.c -lwsock32 -lkernel32
strip -S --strip-unneeded --remove-section=.note.gnu.gold-version --remove-section=.comment --remove-section=.note --remove-section=.note.gnu.build-id --remove-section=.note.ABI-tag bin/netelf.Win32.Mingw64.small.exe
ls -lah bin/netelf.Win32.Mingw64.small.exe

echo Normal.64
x86_64-w64-mingw32-gcc -o bin/netelf.Win32.Mingw64.normal.exe netelf.c -lwsock32 -mconsole 
ls -lah bin/netelf.Win32.Mingw64.normal.exe

echo Small.32
i686-w64-mingw32-gcc $SMALLC_FLAGS -o bin/netelf.Win32.Mingw32.small.exe netelf.c -lkernel32 -lwsock32
strip -S --strip-unneeded --remove-section=.note.gnu.gold-version --remove-section=.comment --remove-section=.note --remove-section=.note.gnu.build-id --remove-section=.note.ABI-tag bin/netelf.Win32.Mingw32.small.exe
ls -lah bin/netelf.Win32.Mingw32.small.exe

echo Normal.32
i686-w64-mingw32-gcc -o bin/netelf.Win32.Mingw32.normal.exe netelf.c -lwsock32 -mconsole 
ls -lah bin/netelf.Win32.Mingw32.normal.exe
