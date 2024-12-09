#endif

struct ib_device *ib_device_get_by_index(u32 ifindex);
void ib_device_put(struct ib_device *device);
/* RDMA device netlink */
void nldev_init(void);
void nldev_exit(void);
