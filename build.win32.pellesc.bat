cc /obin/test.Win32.PellesC.exe /Ze test.c /RELEASE
cc /obin/netelf.Win32.PellesC.normal.exe /Ze /Zd netelf.c /RELEASE /SUBSYSTEM:console
cc /obin/netelf.Win32.PellesC.small.exe /Ze /Zd /DNODEFAULTLIB netelf.c /NODEFAULTLIB /RELEASE /SUBSYSTEM:windows kernel32.lib ws2_32.lib
