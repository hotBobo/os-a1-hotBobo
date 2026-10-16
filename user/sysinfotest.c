#include "kernel/types.h"
#include "kernel/sysinfo.h"
#include "user/user.h"
#include "kernel/fcntl.h"

int failed = 0;

void
check(int ok, char *what)
{
  if (ok) {
    printf("OK   %s\n", what);
  } else {
    printf("FAIL %s\n", what);
    failed = 1;
  }
}

int
main(int argc, char *argv[])
{
  struct sysinfo a, b, c, d, e;

  // Baseline
  check(sysinfo(&a) == 0, "sysinfo returns 0");
  printf("freemem=%lu nproc=%lu nopenfile=%lu\n", a.freemem, a.nproc, a.nopenfile);

  // freemem goes down when memory is allocated
  if (sbrk(10 * 4096) == SBRK_ERROR) {
    fprintf(2, "sysinfotest: sbrk failed\n");
    exit(1);
  }
  sysinfo(&b);
  check(b.freemem <= a.freemem - 10 * 4096, "freemem drops after sbrk");

  // nproc goes up after fork
  int pid = fork();
  if (pid < 0) {
    fprintf(2, "sysinfotest: fork failed\n");
    exit(1);
  }
  if (pid == 0) {
    pause(10);          // stay alive while the parent checks
    exit(0);
  }
  sysinfo(&c);
  check(c.nproc == a.nproc + 1, "nproc +1 after fork");
  wait(0);

  // nopenfile goes up with open, more with pipe, and back down with close
  int fd = open("README", O_RDONLY);
  sysinfo(&d);
  check(d.nopenfile == a.nopenfile + 1, "nopenfile +1 after open");
  close(fd);

  int fds[2];
  pipe(fds);
  sysinfo(&d);
  check(d.nopenfile == a.nopenfile + 2, "nopenfile +2 after pipe");
  close(fds[0]);
  close(fds[1]);
  sysinfo(&e);
  check(e.nopenfile == a.nopenfile, "nopenfile back to baseline after close");

  // an invalid pointer must return -1, not crash the kernel
  check(sysinfo((struct sysinfo *)0xffffffffffULL) == -1, "invalid pointer returns -1");

  if (failed) {
    printf("sysinfotest: SOME TESTS FAILED\n");
    exit(1);
  }
  printf("sysinfotest: ALL OK\n");
  exit(0);
}
