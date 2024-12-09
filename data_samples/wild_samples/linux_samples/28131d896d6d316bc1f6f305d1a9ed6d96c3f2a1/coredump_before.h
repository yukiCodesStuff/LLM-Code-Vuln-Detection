	struct {
		const struct ath10k_mem_region *regions;
		int size;
	} region_table;
};

/* FIXME: where to put this? */
extern unsigned long ath10k_coredump_mask;

#ifdef CONFIG_DEV_COREDUMP

int ath10k_coredump_submit(struct ath10k *ar);
struct ath10k_fw_crash_data *ath10k_coredump_new(struct ath10k *ar);
int ath10k_coredump_create(struct ath10k *ar);
int ath10k_coredump_register(struct ath10k *ar);
void ath10k_coredump_unregister(struct ath10k *ar);
void ath10k_coredump_destroy(struct ath10k *ar);

const struct ath10k_hw_mem_layout *ath10k_coredump_get_mem_layout(struct ath10k *ar);

#else /* CONFIG_DEV_COREDUMP */

static inline int ath10k_coredump_submit(struct ath10k *ar)
{
	return 0;
}
{
	return NULL;
}

#endif /* CONFIG_DEV_COREDUMP */

#endif /* _COREDUMP_H_ */