#include <stdio.h>

int main(argc, argv)
	int argc;
	char **argv;
{
  int i;
  printf("Hello World\n");
  for( i = 0; i < argc; i++ ) {
    printf("%d = %s\n", i, argv[i]);
  }
  return 0;
}
