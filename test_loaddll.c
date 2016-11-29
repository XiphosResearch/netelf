#ifndef _WIN32
typedef void* HMODULE;
#include <dlfcn.h>
#include <stdio.h>
#else
#include <windows.h>
#endif

int main(argc, argv)
	int argc;
	char **argv;
{
	HMODULE library;

	if( argc < 2 ) {
		printf("%s <file.dll>\n", argv[0]);
		return 1;
	}

#ifdef _WIN32
	library = LoadLibrary(argv[1]);
#else
	library = dlopen(argv[1], 0);
#endif
	if( library == NULL ) {
		printf("Error: cannot load library!");
	}

#ifdef _WIN32
	FreeLibrary(library);
#else
	dlclose(library);
#endif

  	return 0;
}
