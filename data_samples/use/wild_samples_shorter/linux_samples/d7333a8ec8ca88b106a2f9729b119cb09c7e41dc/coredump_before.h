void ath10k_coredump_unregister(struct ath10k *ar);
void ath10k_coredump_destroy(struct ath10k *ar);

const struct ath10k_hw_mem_layout *ath10k_coredump_get_mem_layout(struct ath10k *ar);

#else /* CONFIG_DEV_COREDUMP */

	return NULL;
}

#endif /* CONFIG_DEV_COREDUMP */

#endif /* _COREDUMP_H_ */