Taken originally from this post on comp.unix.programmer:
https://groups.google.com/forum/message/raw?msg=comp.unix.programmer/V1M97GBxIXo/6JQtqmpHSsQJ

I ported that to use sockets and leave no file in `/dev/shm`, run with:

	make
	./server /bin/ls &
	./netelf

Then I looked into the source code for glibc and musl to see what goes
on behind the scenes, interesting shit, it executes the file from
`/proc/self/fd/%d` - this could make writing shellcode easier. Combine
with http://aig.cs.man.ac.uk/albums/findsock.c - it then does dup2() to
bind the socket to stdin/out/err so the called executable can just use
stdin & stdout as usual...

See, Glibc:

 * https://github.com/lattera/glibc/blob/a2f34833b1042d5d8eeb263b4cf4caaea138c4ad/sysdeps/unix/sysv/linux/fexecve.c

Musl:

 * https://github.com/bpowers/musl/blob/b27308bed089a4275bfde6f59a84cd8488eaef15/src/internal/procfdname.c
 * https://github.com/bpowers/musl/blob/b27308bed089a4275bfde6f59a84cd8488eaef15/src/process/fexecve.c

To disable this you need to add 'noexec' to /dev/shm mount:

	sudo mount /dev/shm/ -o remount,rw,nosuid,nodev,noexec -t tmpfs

This causes:

	fexecve: Permission denied

probably because the `shm_open` succeeded, but silently the file
descriptor didn't get +x permission, doing fchmod on the handle won't
work either. The file permissions can be checked with `fstat()`.
