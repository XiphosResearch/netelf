cl /nologo /Os /Oy- /Febin/test.Win32.MSVC.%PROCESSOR_ARCHITECTURE%.exe test.c
cl /nologo /Os /Oy- /Febin/test_loaddll.Win32.MSVC.%PROCESSOR_ARCHITECTURE%.exe test_loaddll.c
cl /nologo /Os /Oy- /Oi- /GS- /GA /EHs- /Febin/netelf.Win32.MSVC.%PROCESSOR_ARCHITECTURE%.normal.exe netelf.c /link /SUBSYSTEM:console kernel32.lib wsock32.lib
cl /nologo /Os /Oy- /Oi- /GS- /GA /EHs- /Febin/netelf.Win32.MSVC.%PROCESSOR_ARCHITECTURE%.normal.dll /DDLL netelf.c /link /DLL kernel32.lib wsock32.lib
cl /nologo /Os /Oy- /Oi- /GS- /GA /EHs- /Febin/netelf.Win32.MSVC.%PROCESSOR_ARCHITECTURE%.small.exe /DNODEFAULTLIB netelf.c /link /NODEFAULTLIB /ENTRY:WinMainCRTStartup /SUBSYSTEM:windows kernel32.lib wsock32.lib
cl /nologo /Os /Oy- /Oi- /GS- /GA /EHs- /Febin/netelf.Win32.MSVC.%PROCESSOR_ARCHITECTURE%.small.dll /DDLL /DNODEFAULTLIB netelf.c /link /NODEFAULTLIB /DLL kernel32.lib wsock32.lib
