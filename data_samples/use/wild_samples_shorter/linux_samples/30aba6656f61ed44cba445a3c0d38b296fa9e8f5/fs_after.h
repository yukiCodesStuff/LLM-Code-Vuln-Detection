extern int leases_enable, lease_break_time;
extern int sysctl_protected_symlinks;
extern int sysctl_protected_hardlinks;
extern int sysctl_protected_fifos;
extern int sysctl_protected_regular;

typedef __kernel_rwf_t rwf_t;

struct buffer_head;