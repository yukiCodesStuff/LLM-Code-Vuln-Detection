	if (!size) {
		IWL_DEBUG_FW(trans, "Empty PNVM, skipping.\n");
		ret = -ENOENT;
		goto out;
	}

	IWL_INFO(trans, "loaded PNVM version 0x%0x\n", sha1);

	ret = iwl_trans_set_pnvm(trans, pnvm_data, size);
out:
	kfree(pnvm_data);
	return ret;
}

static int iwl_pnvm_parse(struct iwl_trans *trans, const u8 *data,
			  size_t len)
{
	struct iwl_ucode_tlv *tlv;

	IWL_DEBUG_FW(trans, "Parsing PNVM file\n");

	while (len >= sizeof(*tlv)) {
		u32 tlv_len, tlv_type;

		len -= sizeof(*tlv);
		tlv = (void *)data;

		tlv_len = le32_to_cpu(tlv->length);
		tlv_type = le32_to_cpu(tlv->type);

		if (len < tlv_len) {
			IWL_ERR(trans, "invalid TLV len: %zd/%u\n",
				len, tlv_len);
			return -EINVAL;
		}