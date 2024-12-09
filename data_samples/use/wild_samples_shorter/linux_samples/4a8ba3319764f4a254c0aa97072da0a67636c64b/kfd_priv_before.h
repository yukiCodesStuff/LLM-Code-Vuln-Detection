
	struct kgd2kfd_shared_resources shared_resources;

	void *interrupt_ring;
	size_t interrupt_ring_size;
	atomic_t interrupt_ring_rptr;
	atomic_t interrupt_ring_wptr;
	struct work_struct interrupt_work;
	spinlock_t interrupt_lock;

	/* QCM Device instance */
	struct device_queue_manager *dqm;

	bool init_complete;
	/*
	 * Interrupts of interest to KFD are copied
	 * from the HW ring into a SW ring.
	 */
	bool interrupts_active;
};

/* KGD2KFD callbacks */
void kgd2kfd_exit(void);
	bool is_32bit_user_mode;
};

void kfd_process_create_wq(void);
void kfd_process_destroy_wq(void);
struct kfd_process *kfd_create_process(const struct task_struct *);
struct kfd_process *kfd_get_process(const struct task_struct *);
struct kfd_dev *kfd_topology_enum_kfd_devices(uint8_t idx);

/* Interrupts */
int kfd_interrupt_init(struct kfd_dev *dev);
void kfd_interrupt_exit(struct kfd_dev *dev);
void kgd2kfd_interrupt(struct kfd_dev *kfd, const void *ih_ring_entry);
bool enqueue_ih_ring_entry(struct kfd_dev *kfd,	const void *ih_ring_entry);

/* Power Management */
void kgd2kfd_suspend(struct kfd_dev *kfd);
int kgd2kfd_resume(struct kfd_dev *kfd);