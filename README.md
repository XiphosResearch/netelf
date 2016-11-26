# NetELF

Run the client side of NetELF to download and execute a program over the network from a server. The server sends an arbitrary binary and command-line arguments.

Where possible it will execute the program in-memory, it will not leave files on the filesystem. This makes it ideal for pentests, emergencies and general systems automation.

Originally inspired by a post on [this post on comp.unix.programmer](https://groups.google.com/forum/message/raw?msg=comp.unix.programmer/V1M97GBxIXo/6JQtqmpHSsQJ).

## Supported Platforms

Fully supported & tested:

 * Linux 
 * OpenVMS VAX
 * Ultrix VAX
 * Windows 95+, NT 3.51+

Others that it should work on

 * FreeBSD
 * OSX
 * OSF/1
 * HP-UX
 * QNX
 * z/OS

## Example

```
make
./server.py /bin/ls -la &> /dev/null &
./netelf 127.0.0.1 1337
./netelf 127.0.0.1 1337 
```

## In-memory Execution

I looked into the source code for glibc and musl to see what goes on behind the scenes, interesting, it executes the file from `/proc/self/fd/%d`.

See the following: 
* [Glibc implementation of fexecve](https://github.com/jeremie-koenig/glibc/blob/master-beware-rebase/sysdeps/unix/sysv/linux/fexecve.c)
* [musl implementation of procfdname](https://github.com/esmil/musl/blob/master/src/internal/procfdname.c) 
* [musl implementation of fexecve](https://github.com/esmil/musl/blob/master/src/process/fexecve.c) 

Mount options on tmpfs permiate through to `/proc/self/fd/`, so to disable you need to add `noexec` to `/dev/shm` and other tmpfs mounts:

```
sudo mount /dev/shm/ -o remount,rw,nosuid,nodev,noexec -t tmpfs
```

This causes `fexecve: Permission denied` because the `shm_open` succeeded, but silently the file descriptor didn't get `+x` permission, doing `fchmod` on the handle won't work either. The file permissions can be checked with `fstat`.

Regarding which executables will work with this technique, the most reliable have been self-contained, statically linked executables. In some cases (where the same libc was used on the host used to compile the executable and on the host it is being executed on, and where both have the same libraries/dependencies), dynamically linked executables have worked. Executables which rely on specific environments or external files generally tend to fail.

Furthermore, it is possible to pass arguments to the executable you are running in-memory! The name of the process is derived from `argv[0]`, this can be customised using `--name [kthreadd]`. By default it will use the `basename` of the executable file.
