cc /obin/test.Win32.PellesC.exe /Ze test.c /RELEASE
cc /obin/test_loaddll.Win32.PellesC.exe /Ze test_loaddll.c /RELEASE
cc /obin/netelf.Win32.PellesC.normal.exe /Ze /Zd netelf.c /RELEASE /SUBSYSTEM:console
cc /obin/netelf.Win32.PellesC.normal.dll /Ze /Zd /DDLL netelf.c /RELEASE /DLL
cc /obin/netelf.Win32.PellesC.small.exe /Ze /Zd /DNODEFAULTLIB netelf.c /NODEFAULTLIB /RELEASE /SUBSYSTEM:windows kernel32.lib ws2_32.lib
rem PellesC refuses to find the DLL entry point... TODO: XXX: FIXME: !!!
rem cc /obin/netelf.Win32.PellesC.small.dll /Ze /Zd /DNODEFAULTLIB netelf.c /NODEFAULTLIB /RELEASE /SUBSYSTEM:windows /ENTRY:_DllMainCRTStartup /DLL kernel32.lib ws2_32.lib
