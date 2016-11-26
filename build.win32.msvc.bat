cl /nologo /Os /Oy- /Febin/test.Win32.MSVC.%PROCESSOR_ARCHITECTURE%.exe test.c
cl /nologo /Os /Oy- /Oi- /GS- /GA /EHs- /Febin/netelf.Win32.MSVC.%PROCESSOR_ARCHITECTURE%.normal.exe netelf.c /link /SUBSYSTEM:console kernel32.lib wsock32.lib
cl /nologo /Os /Oy- /Oi- /GS- /GA /EHs- /Febin/netelf.Win32.MSVC.%PROCESSOR_ARCHITECTURE%.small.exe /DNODEFAULTLIB netelf.c /link /NODEFAULTLIB /ENTRY:WinMainCRTStartup /SUBSYSTEM:windows kernel32.lib wsock32.lib
