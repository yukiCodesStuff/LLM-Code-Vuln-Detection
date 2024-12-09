
const struct ath10k_hw_mem_layout *ath10k_coredump_get_mem_layout(struct ath10k *ar)
{
	if (!test_bit(ATH10K_FW_CRASH_DUMP_RAM_DATA, &ath10k_coredump_mask))
		return NULL;

	return _ath10k_coredump_get_mem_layout(ar);
}
EXPORT_SYMBOL(ath10k_coredump_get_mem_layout);

const struct ath10k_hw_mem_layout *_ath10k_coredump_get_mem_layout(struct ath10k *ar)
{
	int i;

	if (WARN_ON(ar->target_version == 0))
		return NULL;

	for (i = 0; i < ARRAY_SIZE(hw_mem_layouts); i++) {

	return NULL;
}

struct ath10k_fw_crash_data *ath10k_coredump_new(struct ath10k *ar)
{
	struct ath10k_fw_crash_data *crash_data = ar->coredump.fw_crash_data;