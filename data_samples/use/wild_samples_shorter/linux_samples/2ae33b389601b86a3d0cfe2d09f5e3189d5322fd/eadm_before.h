	u32 reserved[4];
} __packed;

struct msb {
	u8 fmt:4;
	u8 oc:4;
	u8 flags;
#define OP_STATE_TEMP_ERR	2
#define OP_STATE_PERM_ERR	3

struct scm_driver {
	struct device_driver drv;
	int (*probe) (struct scm_device *scmdev);
	int (*remove) (struct scm_device *scmdev);
	void (*notify) (struct scm_device *scmdev);
	void (*handler) (struct scm_device *scmdev, void *data, int error);
};

int scm_driver_register(struct scm_driver *scmdrv);