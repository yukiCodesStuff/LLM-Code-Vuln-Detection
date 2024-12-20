	for (i = 0; i < hw->region_table.size; i++) {
		size += mem_region->len;
		mem_region++;
	}

	/* reserve space for the headers */
	size += hw->region_table.size * sizeof(struct ath10k_dump_ram_data_hdr);

	/* make sure it is aligned 16 bytes for debug message print out */
	size = ALIGN(size, 16);

	return size;
}

const struct ath10k_hw_mem_layout *ath10k_coredump_get_mem_layout(struct ath10k *ar)
{
	int i;

	if (!test_bit(ATH10K_FW_CRASH_DUMP_RAM_DATA, &ath10k_coredump_mask))
		return NULL;

	if (WARN_ON(ar->target_version == 0))
		return NULL;

	for (i = 0; i < ARRAY_SIZE(hw_mem_layouts); i++) {
		if (ar->target_version == hw_mem_layouts[i].hw_id &&
		    ar->hw_rev == hw_mem_layouts[i].hw_rev &&
		    hw_mem_layouts[i].bus == ar->hif.bus)
			return &hw_mem_layouts[i];
	}

	return NULL;
}
	for (i = 0; i < ARRAY_SIZE(hw_mem_layouts); i++) {
		if (ar->target_version == hw_mem_layouts[i].hw_id &&
		    ar->hw_rev == hw_mem_layouts[i].hw_rev &&
		    hw_mem_layouts[i].bus == ar->hif.bus)
			return &hw_mem_layouts[i];
	}

	return NULL;
}
EXPORT_SYMBOL(ath10k_coredump_get_mem_layout);

struct ath10k_fw_crash_data *ath10k_coredump_new(struct ath10k *ar)
{
	struct ath10k_fw_crash_data *crash_data = ar->coredump.fw_crash_data;

	lockdep_assert_held(&ar->dump_mutex);

	if (ath10k_coredump_mask == 0)
		/* coredump disabled */
		return NULL;

	guid_gen(&crash_data->guid);
	ktime_get_real_ts64(&crash_data->timestamp);

	return crash_data;
}
EXPORT_SYMBOL(ath10k_coredump_new);

static struct ath10k_dump_file_data *ath10k_coredump_build(struct ath10k *ar)
{
	struct ath10k_fw_crash_data *crash_data = ar->coredump.fw_crash_data;
	struct ath10k_ce_crash_hdr *ce_hdr;
	struct ath10k_dump_file_data *dump_data;
	struct ath10k_tlv_dump_data *dump_tlv;
	size_t hdr_len = sizeof(*dump_data);
	size_t len, sofar = 0;
	unsigned char *buf;

	len = hdr_len;

	if (test_bit(ATH10K_FW_CRASH_DUMP_REGISTERS, &ath10k_coredump_mask))
		len += sizeof(*dump_tlv) + sizeof(crash_data->registers);

	if (test_bit(ATH10K_FW_CRASH_DUMP_CE_DATA, &ath10k_coredump_mask))
		len += sizeof(*dump_tlv) + sizeof(*ce_hdr) +
			CE_COUNT * sizeof(ce_hdr->entries[0]);

	if (test_bit(ATH10K_FW_CRASH_DUMP_RAM_DATA, &ath10k_coredump_mask))
		len += sizeof(*dump_tlv) + crash_data->ramdump_buf_len;

	sofar += hdr_len;

	/* This is going to get big when we start dumping FW RAM and such,
	 * so go ahead and use vmalloc.
	 */
	buf = vzalloc(len);
	if (!buf)
		return NULL;

	mutex_lock(&ar->dump_mutex);

	dump_data = (struct ath10k_dump_file_data *)(buf);
	strlcpy(dump_data->df_magic, "ATH10K-FW-DUMP",
		sizeof(dump_data->df_magic));
	dump_data->len = cpu_to_le32(len);

	dump_data->version = cpu_to_le32(ATH10K_FW_CRASH_DUMP_VERSION);

	guid_copy(&dump_data->guid, &crash_data->guid);
	dump_data->chip_id = cpu_to_le32(ar->bus_param.chip_id);
	dump_data->bus_type = cpu_to_le32(0);
	dump_data->target_version = cpu_to_le32(ar->target_version);
	dump_data->fw_version_major = cpu_to_le32(ar->fw_version_major);
	dump_data->fw_version_minor = cpu_to_le32(ar->fw_version_minor);
	dump_data->fw_version_release = cpu_to_le32(ar->fw_version_release);
	dump_data->fw_version_build = cpu_to_le32(ar->fw_version_build);
	dump_data->phy_capability = cpu_to_le32(ar->phy_capability);
	dump_data->hw_min_tx_power = cpu_to_le32(ar->hw_min_tx_power);
	dump_data->hw_max_tx_power = cpu_to_le32(ar->hw_max_tx_power);
	dump_data->ht_cap_info = cpu_to_le32(ar->ht_cap_info);
	dump_data->vht_cap_info = cpu_to_le32(ar->vht_cap_info);
	dump_data->num_rf_chains = cpu_to_le32(ar->num_rf_chains);

	strlcpy(dump_data->fw_ver, ar->hw->wiphy->fw_version,
		sizeof(dump_data->fw_ver));

	dump_data->kernel_ver_code = 0;
	strlcpy(dump_data->kernel_ver, init_utsname()->release,
		sizeof(dump_data->kernel_ver));

	dump_data->tv_sec = cpu_to_le64(crash_data->timestamp.tv_sec);
	dump_data->tv_nsec = cpu_to_le64(crash_data->timestamp.tv_nsec);

	if (test_bit(ATH10K_FW_CRASH_DUMP_REGISTERS, &ath10k_coredump_mask)) {
		dump_tlv = (struct ath10k_tlv_dump_data *)(buf + sofar);
		dump_tlv->type = cpu_to_le32(ATH10K_FW_CRASH_DUMP_REGISTERS);
		dump_tlv->tlv_len = cpu_to_le32(sizeof(crash_data->registers));
		memcpy(dump_tlv->tlv_data, &crash_data->registers,
		       sizeof(crash_data->registers));
		sofar += sizeof(*dump_tlv) + sizeof(crash_data->registers);
	}