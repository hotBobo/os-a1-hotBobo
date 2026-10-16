// Both user and kernel code can use this struct to get information about the system.  The kernel fills it in when the user calls sysinfo(). 24 bytes
struct sysinfo {
  uint64 freemem;   // bytes of free physical memory
  uint64 nproc;     // processes whose state is not UNUSED
  uint64 nopenfile; // entries in the system file table with ref > 0
};
