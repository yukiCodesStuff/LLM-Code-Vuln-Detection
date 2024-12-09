#endif

struct ib_device *ib_device_get_by_index(u32 ifindex);
/* RDMA device netlink */
void nldev_init(void);
void nldev_exit(void);
