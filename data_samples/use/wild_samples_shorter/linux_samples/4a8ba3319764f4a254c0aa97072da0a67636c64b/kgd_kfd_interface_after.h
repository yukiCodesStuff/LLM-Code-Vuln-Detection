	KGD_POOL_FRAMEBUFFER = 3,
};

enum kgd_engine_type {
	KGD_ENGINE_PFP = 1,
	KGD_ENGINE_ME,
	KGD_ENGINE_CE,
	KGD_ENGINE_MEC1,
	KGD_ENGINE_MEC2,
	KGD_ENGINE_RLC,
	KGD_ENGINE_SDMA,
	KGD_ENGINE_MAX
};

struct kgd2kfd_shared_resources {
	/* Bit n == 1 means VMID n is available for KFD. */
	unsigned int compute_vmid_bitmap;

 *
 * @hqd_destroy: Destructs and preempts the queue assigned to that hqd slot.
 *
 * @get_fw_version: Returns FW versions from the header
 *
 * This structure contains function pointers to services that the kgd driver
 * provides to amdkfd driver.
 *
 */
	int (*hqd_load)(struct kgd_dev *kgd, void *mqd, uint32_t pipe_id,
			uint32_t queue_id, uint32_t __user *wptr);

	bool (*hqd_is_occupied)(struct kgd_dev *kgd, uint64_t queue_address,
				uint32_t pipe_id, uint32_t queue_id);

	int (*hqd_destroy)(struct kgd_dev *kgd, uint32_t reset_type,
				unsigned int timeout, uint32_t pipe_id,
				uint32_t queue_id);
	uint16_t (*get_fw_version)(struct kgd_dev *kgd,
				enum kgd_engine_type type);
};

bool kgd2kfd_init(unsigned interface_version,
		  const struct kfd2kgd_calls *f2g,