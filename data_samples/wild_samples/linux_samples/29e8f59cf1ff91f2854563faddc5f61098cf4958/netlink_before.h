struct netlink_tap {
	struct net_device *dev;
	struct module *module;
	struct list_head list;
};

extern int netlink_add_tap(struct netlink_tap *nt);
extern int netlink_remove_tap(struct netlink_tap *nt);

#endif	/* __LINUX_NETLINK_H */