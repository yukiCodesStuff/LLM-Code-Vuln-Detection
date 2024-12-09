			unsigned int naddr, const u8 **addr, u16 *idx,
			u64 *hash, bool sleep_ok)
{
	int i, ret;
	struct fw_vi_mac_cmd cmd, rpl;
	struct fw_vi_mac_exact *p;
	size_t len16;

	if (naddr > ARRAY_SIZE(cmd.u.exact))
		return -EINVAL;
	len16 = DIV_ROUND_UP(offsetof(struct fw_vi_mac_cmd,
				      u.exact[naddr]), 16);

	memset(&cmd, 0, sizeof(cmd));
	cmd.op_to_viid = cpu_to_be32(FW_CMD_OP(FW_VI_MAC_CMD) |
				     FW_CMD_REQUEST |
				     FW_CMD_WRITE |
				     (free ? FW_CMD_EXEC : 0) |
				     FW_VI_MAC_CMD_VIID(viid));
	cmd.freemacs_to_len16 = cpu_to_be32(FW_VI_MAC_CMD_FREEMACS(free) |
					    FW_CMD_LEN16(len16));

	for (i = 0, p = cmd.u.exact; i < naddr; i++, p++) {
		p->valid_to_idx =
			cpu_to_be16(FW_VI_MAC_CMD_VALID |
				    FW_VI_MAC_CMD_IDX(FW_VI_MAC_ADD_MAC));
		memcpy(p->macaddr, addr[i], sizeof(p->macaddr));
	}

	ret = t4vf_wr_mbox_core(adapter, &cmd, sizeof(cmd), &rpl, sleep_ok);
	if (ret)
		return ret;

	for (i = 0, p = rpl.u.exact; i < naddr; i++, p++) {
		u16 index = FW_VI_MAC_CMD_IDX_GET(be16_to_cpu(p->valid_to_idx));

		if (idx)
			idx[i] = (index >= FW_CLS_TCAM_NUM_ENTRIES
				  ? 0xffff
				  : index);
		if (index < FW_CLS_TCAM_NUM_ENTRIES)
			ret++;
		else if (hash)
			*hash |= (1 << hash_mac_addr(addr[i]));
	}
	return ret;
}

/**