	wait_queue_head_t wait;
	struct hid_device *hid;
	struct device *dev;
	spinlock_t list_lock;
	struct list_head list;
};

struct hidraw_report {