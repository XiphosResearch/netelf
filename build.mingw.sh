#!/bin/sh
X32_CC=i686-w64-mingw32-gcc
X64_CC=x86_64-w64-mingw32-gcc
SMALL_CFLAGS="-nostdlib -O2 -s -DNODEFAULTLIB -mwindows -fmerge-all-constants -fno-stack-protector -fno-ident -fomit-frame-pointer -fno-unwind-tables -fno-asynchronous-unwind-tables -ffunction-sections -fdata-sections -flto -Wl,--gc-sections -Wl,--build-id=none"
STRIP_ARGS="-S --strip-unneeded --remove-section=.note.gnu.gold-version --remove-section=.comment --remove-section=.note --remove-section=.note.gnu.build-id --remove-section=.note.ABI-tag"

echo Test.64.exe
EXE=bin/test.win64.mingw.exe
$X64_CC -o $EXE test.c -lwsock32 -mconsole 
ls -lah $EXE
echo ""

echo Test.32.exe
EXE=bin/test.win32.mingw.exe
$X32_CC -o $EXE test.c -lwsock32 -mconsole 
ls -lah $EXE
echo ""

echo Test_LoadDLL.64.exe
EXE=bin/test_loaddll.win64.mingw.exe
$X64_CC -o $EXE test_loaddll.c -lwsock32 -mconsole 
ls -lah $EXE
echo ""

echo Test_LoadDLL.32.exe
EXE=bin/test_loaddll.win32.mingw.exe
$X32_CC -o $EXE test_loaddll.c -lwsock32 -mconsole 
ls -lah $EXE
echo ""

echo Small.64.exe
EXE=bin/netelf.win64.mingw.small.exe
$X64_CC $SMALL_CFLAGS -o $EXE netelf.c -lwsock32 -lkernel32
strip $STRIP_ARGS $EXE
ls -lah $EXE
echo ""

echo Normal.64.exe
EXE=bin/netelf.win64.mingw.normal.exe
$X64_CC -o $EXE netelf.c -lwsock32 -mconsole 
ls -lah $EXE
echo ""

echo Small.64.dll
DLL=bin/netelf.win64.mingw.small.dll
$X64_CC $SMALL_CFLAGS -o $DLL -DDLL -shared netelf.c -lwsock32 -lkernel32
ls -lah $DLL
echo ""

echo Normal.64.dll
DLL=bin/netelf.win64.mingw.normal.dll
$X64_CC -o $DLL -DDLL -shared netelf.c -lwsock32
ls -lah $DLL
echo ""

echo Small.32.exe
EXE=bin/netelf.win32.mingw.small.exe
$X32_CC $SMALL_CFLAGS -o $EXE netelf.c -lkernel32 -lwsock32
strip $STRIP_ARGS $EXE
ls -lah $EXE
echo ""

echo Normal.32.exe
EXE=bin/netelf.win32.mingw.normal.exe
$X32_CC -o $EXE netelf.c -lwsock32 -mconsole
ls -lah $EXE
echo ""

echo Small.32.dll
DLL=bin/netelf.win32.mingw.small.dll
$X32_CC $SMALL_CFLAGS -o $DLL -DDLL -shared netelf.c -lwsock32 -lkernel32
ls -lah $DLL
echo ""

echo Normal.32.dll
DLL=bin/netelf.win32.mingw.normal.dll
$X32_CC -o $DLL -DDLL -shared netelf.c -lwsock32
ls -lah $DLL
echo ""