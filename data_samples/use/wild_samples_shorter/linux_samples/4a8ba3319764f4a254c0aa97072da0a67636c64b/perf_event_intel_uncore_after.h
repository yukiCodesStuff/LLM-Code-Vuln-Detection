#define UNCORE_PCI_DEV_TYPE(data)	((data >> 8) & 0xff)
#define UNCORE_PCI_DEV_IDX(data)	(data & 0xff)
#define UNCORE_EXTRA_PCI_DEV		0xff
#define UNCORE_EXTRA_PCI_DEV_MAX	3

/* support up to 8 sockets */
#define UNCORE_SOCKET_MAX		8
