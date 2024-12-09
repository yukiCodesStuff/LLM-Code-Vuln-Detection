
	setup_rfi_flush(types, enable);
	setup_count_cache_flush();
}

#ifdef CONFIG_PCI_IOV
enum rtas_iov_fw_value_map {