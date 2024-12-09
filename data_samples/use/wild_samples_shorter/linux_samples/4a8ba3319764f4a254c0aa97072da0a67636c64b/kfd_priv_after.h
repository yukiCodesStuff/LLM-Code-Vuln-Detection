
	struct kgd2kfd_shared_resources shared_resources;

	/* QCM Device instance */
	struct device_queue_manager *dqm;

	bool init_complete;
};

/* KGD2KFD callbacks */
void kgd2kfd_exit(void);
	bool is_32bit_user_mode;
};

/**
 * Ioctl function type.
 *
 * \param filep pointer to file structure.
 * \param p amdkfd process pointer.
 * \param data pointer to arg that was copied from user.
 */
typedef int amdkfd_ioctl_t(struct file *filep, struct kfd_process *p,
				void *data);

struct amdkfd_ioctl_desc {
	unsigned int cmd;
	int flags;
	amdkfd_ioctl_t *func;
	unsigned int cmd_drv;
	const char *name;
};

void kfd_process_create_wq(void);
void kfd_process_destroy_wq(void);
struct kfd_process *kfd_create_process(const struct task_struct *);
struct kfd_process *kfd_get_process(const struct task_struct *);
struct kfd_dev *kfd_topology_enum_kfd_devices(uint8_t idx);

/* Interrupts */
void kgd2kfd_interrupt(struct kfd_dev *kfd, const void *ih_ring_entry);

/* Power Management */
void kgd2kfd_suspend(struct kfd_dev *kfd);
int kgd2kfd_resume(struct kfd_dev *kfd);