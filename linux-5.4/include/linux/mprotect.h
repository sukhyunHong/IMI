#ifndef _LINUX_MPROTECT_H
#define _LINUX_MPROTECT_H

extern int do_mprotect_pkey(unsigned long start, size_t len,
		unsigned long prot, int pkey);

#endif /* _LINUX_MOUNT_H */