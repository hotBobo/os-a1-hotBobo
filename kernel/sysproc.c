#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
#include "vm.h"
#include "sysinfo.h"

uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
  kexit(n);
  return 0; // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return kfork();
}

uint64
sys_wait(void)
{
  uint64 p;
  argaddr(0, &p);
  return kwait(p);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int t;
  int n;

  argint(0, &n);
  argint(1, &t);
  addr = myproc()->sz;

  if (t == SBRK_EAGER || n < 0) {
    if (growproc(n) < 0) {
      return -1;
    }
  } else {
    // Lazily allocate memory for this process: increase its memory
    // size but don't allocate memory. If the processes uses the
    // memory, vmfault() will allocate it.
    if (addr + n < addr)
      return -1;
    if (addr + n > TRAPFRAME)
      return -1;
    myproc()->sz += n;
  }
  return addr;
}

uint64
sys_pause(void)
{
  int n;
  uint ticks0;

  argint(0, &n);
  if (n < 0)
    n = 0;
  acquire(&tickslock);
  ticks0 = ticks;
  while (ticks - ticks0 < n) {
    if (killed(myproc())) {
      release(&tickslock);
      return -1;
    }
    sleep_prepare(&ticks);
    release(&tickslock);
    sleep();
    acquire(&tickslock);
  }
  release(&tickslock);
  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  argint(0, &pid);
  return kkill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

uint64
sys_trace(void)
{
  argint(0, &myproc()->tracemask);
  return 0;
}

uint64
sys_sysinfo(void) {
    uint64 addr;
    // the user's pointer: just a number in user address space, we pass into a0 when invoking sysinfo in user code
    argaddr(0, &addr);            // read from a0 into addr, which is a pointer to a struct sysinfo in user space
    struct sysinfo info;          // a kernel-side copy, separate from the user-side struct sysinfo, to be filled in by the kernel
    // Fill in the struct with information about the system
    info.freemem   = kfreemem(); // bytes of free physical memory
    info.nproc     = countproc(); // processes whose state is not UNUSED
    info.nopenfile = countfiles(); // entries in the system file table with ref > 0
    // If the copyout fails, return -1. Otherwise, return 0 to indicate success.
    // Copy the filled-in struct from kernel space to user space at the address provided by the user.
    // Pagetable is translation for where the user virtual address is in physical memory
    // sz is the size of the process's memory, addr is the user virtual address
    // (char *)&info is the kernel address of the struct sysinfo (to copy from), sizeof(info) is the size of the struct
    if (copyout(myproc()->pagetable, myproc()->sz, addr, (char *)&info, sizeof(info)) < 0) {
        return -1;
    }
    return 0;
}

uint64
sys_getprocs(void) {
  uint64 addr; int max;
  argaddr(0, &addr);
  argint(1, &max);
  return getprocs(addr, max);
}