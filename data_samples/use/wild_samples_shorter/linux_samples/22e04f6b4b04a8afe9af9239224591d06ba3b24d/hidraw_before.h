	wait_queue_head_t wait;
	struct hid_device *hid;
	struct device *dev;
	struct list_head list;
};

struct hidraw_report {