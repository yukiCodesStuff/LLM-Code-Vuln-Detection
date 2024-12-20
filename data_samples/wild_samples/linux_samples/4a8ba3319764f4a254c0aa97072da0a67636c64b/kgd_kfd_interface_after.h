enum kgd_memory_pool {
	KGD_POOL_SYSTEM_CACHEABLE = 1,
	KGD_POOL_SYSTEM_WRITECOMBINE = 2,
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

	/* Compute pipes are counted starting from MEC0/pipe0 as 0. */
	unsigned int first_compute_pipe;

	/* Number of MEC pipes available for KFD. */
	unsigned int compute_pipe_count;

	/* Base address of doorbell aperture. */
	phys_addr_t doorbell_physical_address;

	/* Size in bytes of doorbell aperture. */
	size_t doorbell_aperture_size;

	/* Number of bytes at start of aperture reserved for KGD. */
	size_t doorbell_start_offset;
};

/**
 * struct kgd2kfd_calls
 *
 * @exit: Notifies amdkfd that kgd module is unloaded
 *
 * @probe: Notifies amdkfd about a probe done on a device in the kgd driver.
 *
 * @device_init: Initialize the newly probed device (if it is a device that
 * amdkfd supports)
 *
 * @device_exit: Notifies amdkfd about a removal of a kgd device
 *
 * @suspend: Notifies amdkfd about a suspend action done to a kgd device
 *
 * @resume: Notifies amdkfd about a resume action done to a kgd device
 *
 * This structure contains function callback pointers so the kgd driver
 * will notify to the amdkfd about certain status changes.
 *
 */
struct kgd2kfd_calls {
	void (*exit)(void);
	struct kfd_dev* (*probe)(struct kgd_dev *kgd, struct pci_dev *pdev);
	bool (*device_init)(struct kfd_dev *kfd,
			const struct kgd2kfd_shared_resources *gpu_resources);
	void (*device_exit)(struct kfd_dev *kfd);
	void (*interrupt)(struct kfd_dev *kfd, const void *ih_ring_entry);
	void (*suspend)(struct kfd_dev *kfd);
	int (*resume)(struct kfd_dev *kfd);
};

/**
 * struct kfd2kgd_calls
 *
 * @init_sa_manager: Initialize an instance of the sa manager, used by
 * amdkfd for all system memory allocations that are mapped to the GART
 * address space
 *
 * @fini_sa_manager: Releases all memory allocations for amdkfd that are
 * handled by kgd sa manager
 *
 * @allocate_mem: Allocate a buffer from amdkfd's sa manager. The buffer can
 * be used for mqds, hpds, kernel queue, fence and runlists
 *
 * @free_mem: Frees a buffer that was allocated by amdkfd's sa manager
 *
 * @get_vmem_size: Retrieves (physical) size of VRAM
 *
 * @get_gpu_clock_counter: Retrieves GPU clock counter
 *
 * @get_max_engine_clock_in_mhz: Retrieves maximum GPU clock in MHz
 *
 * @program_sh_mem_settings: A function that should initiate the memory
 * properties such as main aperture memory type (cache / non cached) and
 * secondary aperture base address, size and memory type.
 * This function is used only for no cp scheduling mode.
 *
 * @set_pasid_vmid_mapping: Exposes pasid/vmid pair to the H/W for no cp
 * scheduling mode. Only used for no cp scheduling mode.
 *
 * @init_memory: Initializes memory apertures to fixed base/limit address
 * and non cached memory types.
 *
 * @init_pipeline: Initialized the compute pipelines.
 *
 * @hqd_load: Loads the mqd structure to a H/W hqd slot. used only for no cp
 * sceduling mode.
 *
 * @hqd_is_occupies: Checks if a hqd slot is occupied.
 *
 * @hqd_destroy: Destructs and preempts the queue assigned to that hqd slot.
 *
 * @get_fw_version: Returns FW versions from the header
 *
 * This structure contains function pointers to services that the kgd driver
 * provides to amdkfd driver.
 *
 */
struct kfd2kgd_calls {
	/* Memory management. */
	int (*init_sa_manager)(struct kgd_dev *kgd, unsigned int size);
	void (*fini_sa_manager)(struct kgd_dev *kgd);
	int (*allocate_mem)(struct kgd_dev *kgd, size_t size, size_t alignment,
			enum kgd_memory_pool pool, struct kgd_mem **mem);

	void (*free_mem)(struct kgd_dev *kgd, struct kgd_mem *mem);

	uint64_t (*get_vmem_size)(struct kgd_dev *kgd);
	uint64_t (*get_gpu_clock_counter)(struct kgd_dev *kgd);

	uint32_t (*get_max_engine_clock_in_mhz)(struct kgd_dev *kgd);

	/* Register access functions */
	void (*program_sh_mem_settings)(struct kgd_dev *kgd, uint32_t vmid,
			uint32_t sh_mem_config,	uint32_t sh_mem_ape1_base,
			uint32_t sh_mem_ape1_limit, uint32_t sh_mem_bases);

	int (*set_pasid_vmid_mapping)(struct kgd_dev *kgd, unsigned int pasid,
					unsigned int vmid);

	int (*init_memory)(struct kgd_dev *kgd);
	int (*init_pipeline)(struct kgd_dev *kgd, uint32_t pipe_id,
				uint32_t hpd_size, uint64_t hpd_gpu_addr);

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
		  const struct kgd2kfd_calls **g2f);

#endif /* KGD_KFD_INTERFACE_H_INCLUDED */
struct kgd2kfd_calls {
	void (*exit)(void);
	struct kfd_dev* (*probe)(struct kgd_dev *kgd, struct pci_dev *pdev);
	bool (*device_init)(struct kfd_dev *kfd,
			const struct kgd2kfd_shared_resources *gpu_resources);
	void (*device_exit)(struct kfd_dev *kfd);
	void (*interrupt)(struct kfd_dev *kfd, const void *ih_ring_entry);
	void (*suspend)(struct kfd_dev *kfd);
	int (*resume)(struct kfd_dev *kfd);
};

/**
 * struct kfd2kgd_calls
 *
 * @init_sa_manager: Initialize an instance of the sa manager, used by
 * amdkfd for all system memory allocations that are mapped to the GART
 * address space
 *
 * @fini_sa_manager: Releases all memory allocations for amdkfd that are
 * handled by kgd sa manager
 *
 * @allocate_mem: Allocate a buffer from amdkfd's sa manager. The buffer can
 * be used for mqds, hpds, kernel queue, fence and runlists
 *
 * @free_mem: Frees a buffer that was allocated by amdkfd's sa manager
 *
 * @get_vmem_size: Retrieves (physical) size of VRAM
 *
 * @get_gpu_clock_counter: Retrieves GPU clock counter
 *
 * @get_max_engine_clock_in_mhz: Retrieves maximum GPU clock in MHz
 *
 * @program_sh_mem_settings: A function that should initiate the memory
 * properties such as main aperture memory type (cache / non cached) and
 * secondary aperture base address, size and memory type.
 * This function is used only for no cp scheduling mode.
 *
 * @set_pasid_vmid_mapping: Exposes pasid/vmid pair to the H/W for no cp
 * scheduling mode. Only used for no cp scheduling mode.
 *
 * @init_memory: Initializes memory apertures to fixed base/limit address
 * and non cached memory types.
 *
 * @init_pipeline: Initialized the compute pipelines.
 *
 * @hqd_load: Loads the mqd structure to a H/W hqd slot. used only for no cp
 * sceduling mode.
 *
 * @hqd_is_occupies: Checks if a hqd slot is occupied.
 *
 * @hqd_destroy: Destructs and preempts the queue assigned to that hqd slot.
 *
 * @get_fw_version: Returns FW versions from the header
 *
 * This structure contains function pointers to services that the kgd driver
 * provides to amdkfd driver.
 *
 */
struct kfd2kgd_calls {
	/* Memory management. */
	int (*init_sa_manager)(struct kgd_dev *kgd, unsigned int size);
	void (*fini_sa_manager)(struct kgd_dev *kgd);
	int (*allocate_mem)(struct kgd_dev *kgd, size_t size, size_t alignment,
			enum kgd_memory_pool pool, struct kgd_mem **mem);

	void (*free_mem)(struct kgd_dev *kgd, struct kgd_mem *mem);

	uint64_t (*get_vmem_size)(struct kgd_dev *kgd);
	uint64_t (*get_gpu_clock_counter)(struct kgd_dev *kgd);

	uint32_t (*get_max_engine_clock_in_mhz)(struct kgd_dev *kgd);

	/* Register access functions */
	void (*program_sh_mem_settings)(struct kgd_dev *kgd, uint32_t vmid,
			uint32_t sh_mem_config,	uint32_t sh_mem_ape1_base,
			uint32_t sh_mem_ape1_limit, uint32_t sh_mem_bases);

	int (*set_pasid_vmid_mapping)(struct kgd_dev *kgd, unsigned int pasid,
					unsigned int vmid);

	int (*init_memory)(struct kgd_dev *kgd);
	int (*init_pipeline)(struct kgd_dev *kgd, uint32_t pipe_id,
				uint32_t hpd_size, uint64_t hpd_gpu_addr);

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
		  const struct kgd2kfd_calls **g2f);

#endif /* KGD_KFD_INTERFACE_H_INCLUDED */
struct kfd2kgd_calls {
	/* Memory management. */
	int (*init_sa_manager)(struct kgd_dev *kgd, unsigned int size);
	void (*fini_sa_manager)(struct kgd_dev *kgd);
	int (*allocate_mem)(struct kgd_dev *kgd, size_t size, size_t alignment,
			enum kgd_memory_pool pool, struct kgd_mem **mem);

	void (*free_mem)(struct kgd_dev *kgd, struct kgd_mem *mem);

	uint64_t (*get_vmem_size)(struct kgd_dev *kgd);
	uint64_t (*get_gpu_clock_counter)(struct kgd_dev *kgd);

	uint32_t (*get_max_engine_clock_in_mhz)(struct kgd_dev *kgd);

	/* Register access functions */
	void (*program_sh_mem_settings)(struct kgd_dev *kgd, uint32_t vmid,
			uint32_t sh_mem_config,	uint32_t sh_mem_ape1_base,
			uint32_t sh_mem_ape1_limit, uint32_t sh_mem_bases);

	int (*set_pasid_vmid_mapping)(struct kgd_dev *kgd, unsigned int pasid,
					unsigned int vmid);

	int (*init_memory)(struct kgd_dev *kgd);
	int (*init_pipeline)(struct kgd_dev *kgd, uint32_t pipe_id,
				uint32_t hpd_size, uint64_t hpd_gpu_addr);

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
		  const struct kgd2kfd_calls **g2f);

#endif /* KGD_KFD_INTERFACE_H_INCLUDED */