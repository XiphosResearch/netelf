Taken originally from [this post on comp.unix.programmer](comp.unix.programmer)

I ported that to use sockets and leave no file in `/dev/shm`, run with:

```
make
./server /bin/ls &
./netelf
```

Then I looked into the source code for glibc and musl to see what goes on behind the scenes, interesting shit, it executes the file from `/proc/self/fd/%d` - this could make writing shellcode easier. Combine with [findsock.c](findsock.c) - it then does dup2() to bind the socket to stdin/out/err so the called executable can just use stdin & stdout as usual...

See the following: 
* [Glibc implementation of fexecve](glibc-fexecve)
* [muslibc implementation of procfdname](musl-procfdname) 
* [muslibc implementation of fexecve](musl-fexecve) 

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

A further caveat is that as of this time, it is not possible to supply commandline arguments to the executable being executed. This will be fixed in a future release.


[comp.unix.programmer]: https://groups.google.com/forum/message/raw?msg=comp.unix.programmer/V1M97GBxIXo/6JQtqmpHSsQJ
[findsock.c]: http://aig.cs.man.ac.uk/albums/findsock.c
[glibc-fexecve]: https://github.com/lattera/glibc/blob/a2f34833b1042d5d8eeb263b4cf4caaea138c4ad/sysdeps/unix/sysv/linux/fexecve.c
[musl-procfdname]: https://github.com/bpowers/musl/blob/b27308bed089a4275bfde6f59a84cd8488eaef15/src/internal/procfdname.c
[musl-fexecve]: https://github.com/bpowers/musl/blob/b27308bed089a4275bfde6f59a84cd8488eaef15/src/process/fexecve.c
