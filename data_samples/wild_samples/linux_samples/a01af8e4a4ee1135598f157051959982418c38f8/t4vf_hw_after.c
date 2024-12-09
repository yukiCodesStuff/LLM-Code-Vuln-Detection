{
	struct fw_vi_rxmode_cmd cmd;

	/* convert to FW values */
	if (mtu < 0)
		mtu = FW_VI_RXMODE_CMD_MTU_MASK;
	if (promisc < 0)
		promisc = FW_VI_RXMODE_CMD_PROMISCEN_MASK;
	if (all_multi < 0)
		all_multi = FW_VI_RXMODE_CMD_ALLMULTIEN_MASK;
	if (bcast < 0)
		bcast = FW_VI_RXMODE_CMD_BROADCASTEN_MASK;
	if (vlanex < 0)
		vlanex = FW_VI_RXMODE_CMD_VLANEXEN_MASK;

	memset(&cmd, 0, sizeof(cmd));
	cmd.op_to_viid = cpu_to_be32(FW_CMD_OP(FW_VI_RXMODE_CMD) |
				     FW_CMD_REQUEST |
				     FW_CMD_WRITE |
				     FW_VI_RXMODE_CMD_VIID(viid));
	cmd.retval_len16 = cpu_to_be32(FW_LEN16(cmd));
	cmd.mtu_to_vlanexen =
		cpu_to_be32(FW_VI_RXMODE_CMD_MTU(mtu) |
			    FW_VI_RXMODE_CMD_PROMISCEN(promisc) |
			    FW_VI_RXMODE_CMD_ALLMULTIEN(all_multi) |
			    FW_VI_RXMODE_CMD_BROADCASTEN(bcast) |
			    FW_VI_RXMODE_CMD_VLANEXEN(vlanex));
	return t4vf_wr_mbox_core(adapter, &cmd, sizeof(cmd), NULL, sleep_ok);
}

/**
 *	t4vf_alloc_mac_filt - allocates exact-match filters for MAC addresses
 *	@adapter: the adapter
 *	@viid: the Virtual Interface Identifier
 *	@free: if true any existing filters for this VI id are first removed
 *	@naddr: the number of MAC addresses to allocate filters for (up to 7)
 *	@addr: the MAC address(es)
 *	@idx: where to store the index of each allocated filter
 *	@hash: pointer to hash address filter bitmap
 *	@sleep_ok: call is allowed to sleep
 *
 *	Allocates an exact-match filter for each of the supplied addresses and
 *	sets it to the corresponding address.  If @idx is not %NULL it should
 *	have at least @naddr entries, each of which will be set to the index of
 *	the filter allocated for the corresponding MAC address.  If a filter
 *	could not be allocated for an address its index is set to 0xffff.
 *	If @hash is not %NULL addresses that fail to allocate an exact filter
 *	are hashed and update the hash filter bitmap pointed at by @hash.
 *
 *	Returns a negative error number or the number of filters allocated.
 */
int t4vf_alloc_mac_filt(struct adapter *adapter, unsigned int viid, bool free,
			unsigned int naddr, const u8 **addr, u16 *idx,
			u64 *hash, bool sleep_ok)
{
	int offset, ret = 0;
	unsigned nfilters = 0;
	unsigned int rem = naddr;
	struct fw_vi_mac_cmd cmd, rpl;

	if (naddr > FW_CLS_TCAM_NUM_ENTRIES)
		return -EINVAL;

	for (offset = 0; offset < naddr; /**/) {
		unsigned int fw_naddr = (rem < ARRAY_SIZE(cmd.u.exact)
					 ? rem
					 : ARRAY_SIZE(cmd.u.exact));
		size_t len16 = DIV_ROUND_UP(offsetof(struct fw_vi_mac_cmd,
						     u.exact[fw_naddr]), 16);
		struct fw_vi_mac_exact *p;
		int i;

		memset(&cmd, 0, sizeof(cmd));
		cmd.op_to_viid = cpu_to_be32(FW_CMD_OP(FW_VI_MAC_CMD) |
					     FW_CMD_REQUEST |
					     FW_CMD_WRITE |
					     (free ? FW_CMD_EXEC : 0) |
					     FW_VI_MAC_CMD_VIID(viid));
		cmd.freemacs_to_len16 =
			cpu_to_be32(FW_VI_MAC_CMD_FREEMACS(free) |
				    FW_CMD_LEN16(len16));

		for (i = 0, p = cmd.u.exact; i < fw_naddr; i++, p++) {
			p->valid_to_idx = cpu_to_be16(
				FW_VI_MAC_CMD_VALID |
				FW_VI_MAC_CMD_IDX(FW_VI_MAC_ADD_MAC));
			memcpy(p->macaddr, addr[offset+i], sizeof(p->macaddr));
		}


		ret = t4vf_wr_mbox_core(adapter, &cmd, sizeof(cmd), &rpl,
					sleep_ok);
		if (ret && ret != -ENOMEM)
			break;

		for (i = 0, p = rpl.u.exact; i < fw_naddr; i++, p++) {
			u16 index = FW_VI_MAC_CMD_IDX_GET(
				be16_to_cpu(p->valid_to_idx));

			if (idx)
				idx[offset+i] =
					(index >= FW_CLS_TCAM_NUM_ENTRIES
					 ? 0xffff
					 : index);
			if (index < FW_CLS_TCAM_NUM_ENTRIES)
				nfilters++;
			else if (hash)
				*hash |= (1ULL << hash_mac_addr(addr[offset+i]));
		}

		free = false;
		offset += fw_naddr;
		rem -= fw_naddr;
	}

	/*
	 * If there were no errors or we merely ran out of room in our MAC
	 * address arena, return the number of filters actually written.
	 */
	if (ret == 0 || ret == -ENOMEM)
		ret = nfilters;
	return ret;
}