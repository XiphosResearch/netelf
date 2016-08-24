Taken originally from [this post on comp.unix.programmer](https://groups.google.com/forum/message/raw?msg=comp.unix.programmer/V1M97GBxIXo/6JQtqmpHSsQJ)

I ported that to use sockets and leave no file in `/dev/shm`, run with:

```
make
./server.py 127.0.0.1 1337 /bin/ls ls -la &
./netelf 127.0.0.1 1337 
```

Then I looked into the source code for glibc and musl to see what goes on behind the scenes, interesting shit, it executes the file from `/proc/self/fd/%d` - this could make writing shellcode easier. Combine with [findsock.c](http://aig.cs.man.ac.uk/albums/findsock.c) - it then does dup2() to bind the socket to stdin/out/err so the called executable can just use stdin & stdout as usual...

See the following: 
* [Glibc implementation of fexecve](https://github.com/jeremie-koenig/glibc/blob/master-beware-rebase/sysdeps/unix/sysv/linux/fexecve.c)
* [musl implementation of procfdname](https://github.com/esmil/musl/blob/master/src/internal/procfdname.c) 
* [musl implementation of fexecve](https://github.com/esmil/musl/blob/master/src/process/fexecve.c) 

To disable this you need to add 'noexec' to /dev/shm mount:

```
sudo mount /dev/shm/ -o remount,rw,nosuid,nodev,noexec -t tmpfs
```

This causes:
```
fexecve: Permission denied
```
probably because the `shm_open` succeeded, but silently the file descriptor didn't get +x permission, doing fchmod on the handle won't work either. The file permissions can be checked with `fstat()`.

Regarding which executables will work with this technique, the most reliable have been self-contained, statically linked executables. In some cases (where the same libc was used on the host used to compile the executable and on the host it is being executed on, and where both have the same libraries/dependencies), dynamically linked executables have worked. Executables which rely on specific environments or external files generally tend to fail.

Furthermore, it is possible to pass arguments to the executable you are running in-memory! You MUST pass at least one (argv[0]), which you can just set to anything, in the server.py currently. In the example above, this is "ls". Further arguments will also be passed along.
