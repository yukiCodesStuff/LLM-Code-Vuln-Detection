#define __NR_bpf		351
#define __NR_s390_pci_mmio_write	352
#define __NR_s390_pci_mmio_read		353
#define NR_syscalls 354

/* 
 * There are some system calls that are not present on 64 bit, some
 * have a different name although they do the same (e.g. __NR_chown32