 * @IWL_DBG_TLV_TYPE_HCMD: host command TLV
 * @IWL_DBG_TLV_TYPE_REGION: region TLV
 * @IWL_DBG_TLV_TYPE_TRIGGER: trigger TLV
 * @IWL_DBG_TLV_TYPE_CONF_SET: conf set TLV
 * @IWL_DBG_TLV_TYPE_NUM: number of debug TLVs
 */
enum iwl_dbg_tlv_type {
	IWL_DBG_TLV_TYPE_DEBUG_INFO =
	IWL_DBG_TLV_TYPE_HCMD,
	IWL_DBG_TLV_TYPE_REGION,
	IWL_DBG_TLV_TYPE_TRIGGER,
	IWL_DBG_TLV_TYPE_CONF_SET,
	IWL_DBG_TLV_TYPE_NUM,
};

/**
	[IWL_DBG_TLV_TYPE_HCMD]		= {.min_ver = 1, .max_ver = 1,},
	[IWL_DBG_TLV_TYPE_REGION]	= {.min_ver = 1, .max_ver = 2,},
	[IWL_DBG_TLV_TYPE_TRIGGER]	= {.min_ver = 1, .max_ver = 1,},
	[IWL_DBG_TLV_TYPE_CONF_SET]	= {.min_ver = 1, .max_ver = 1,},
};

static int iwl_dbg_tlv_add(const struct iwl_ucode_tlv *tlv,
			   struct list_head *list)
	return ret;
}

static int iwl_dbg_tlv_config_set(struct iwl_trans *trans,
				  const struct iwl_ucode_tlv *tlv)
{
	struct iwl_fw_ini_conf_set_tlv *conf_set = (void *)tlv->data;
	u32 tp = le32_to_cpu(conf_set->time_point);
	u32 type = le32_to_cpu(conf_set->set_type);

	if (tp <= IWL_FW_INI_TIME_POINT_INVALID ||
	    tp >= IWL_FW_INI_TIME_POINT_NUM) {
		IWL_DEBUG_FW(trans,
			     "WRT: Invalid time point %u for config set TLV\n", tp);
		return -EINVAL;
	}

	if (type <= IWL_FW_INI_CONFIG_SET_TYPE_INVALID ||
	    type >= IWL_FW_INI_CONFIG_SET_TYPE_MAX_NUM) {
		IWL_DEBUG_FW(trans,
			     "WRT: Invalid config set type %u for config set TLV\n", type);
		return -EINVAL;
	}

	return iwl_dbg_tlv_add(tlv, &trans->dbg.time_point[tp].config_list);
}

static int (*dbg_tlv_alloc[])(struct iwl_trans *trans,
			      const struct iwl_ucode_tlv *tlv) = {
	[IWL_DBG_TLV_TYPE_DEBUG_INFO]	= iwl_dbg_tlv_alloc_debug_info,
	[IWL_DBG_TLV_TYPE_BUF_ALLOC]	= iwl_dbg_tlv_alloc_buf_alloc,
	[IWL_DBG_TLV_TYPE_HCMD]		= iwl_dbg_tlv_alloc_hcmd,
	[IWL_DBG_TLV_TYPE_REGION]	= iwl_dbg_tlv_alloc_region,
	[IWL_DBG_TLV_TYPE_TRIGGER]	= iwl_dbg_tlv_alloc_trigger,
	[IWL_DBG_TLV_TYPE_CONF_SET]	= iwl_dbg_tlv_config_set,
};

void iwl_dbg_tlv_alloc(struct iwl_trans *trans, const struct iwl_ucode_tlv *tlv,
		       bool ext)
			list_del(&tlv_node->list);
			kfree(tlv_node);
		}

		list_for_each_entry_safe(tlv_node, tlv_node_tmp,
					 &tp->config_list, list) {
			list_del(&tlv_node->list);
			kfree(tlv_node);
		}

	}

	for (i = 0; i < ARRAY_SIZE(trans->dbg.fw_mon_ini); i++)
		iwl_dbg_tlv_fragments_free(trans, i);
		INIT_LIST_HEAD(&tp->trig_list);
		INIT_LIST_HEAD(&tp->hcmd_list);
		INIT_LIST_HEAD(&tp->active_trig_list);
		INIT_LIST_HEAD(&tp->config_list);
	}
}

static int iwl_dbg_tlv_alloc_fragment(struct iwl_fw_runtime *fwrt,
{
	int ret, i;

	if (fw_has_capa(&fwrt->fw->ucode_capa,
			IWL_UCODE_TLV_CAPA_DRAM_FRAG_SUPPORT))
		return;

	for (i = 0; i < IWL_FW_INI_ALLOCATION_NUM; i++) {
		ret = iwl_dbg_tlv_apply_buffer(fwrt, i);
		if (ret)
			IWL_WARN(fwrt,
	}
}

static int iwl_dbg_tlv_update_dram(struct iwl_fw_runtime *fwrt,
				   enum iwl_fw_ini_allocation_id alloc_id,
				   struct iwl_dram_info *dram_info)
{
	struct iwl_fw_mon *fw_mon;
	u32 remain_frags, num_frags;
	int j, fw_mon_idx = 0;
	struct iwl_buf_alloc_cmd *data;

	if (le32_to_cpu(fwrt->trans->dbg.fw_mon_cfg[alloc_id].buf_location) !=
			IWL_FW_INI_LOCATION_DRAM_PATH) {
		IWL_DEBUG_FW(fwrt, "DRAM_PATH is not supported alloc_id %u\n", alloc_id);
		return -1;
	}

	fw_mon = &fwrt->trans->dbg.fw_mon_ini[alloc_id];

	/* the first fragment of DBGC1 is given to the FW via register
	 * or context info
	 */
	if (alloc_id == IWL_FW_INI_ALLOCATION_ID_DBGC1)
		fw_mon_idx++;

	remain_frags = fw_mon->num_frags - fw_mon_idx;
	if (!remain_frags)
		return -1;

	num_frags = min_t(u32, remain_frags, BUF_ALLOC_MAX_NUM_FRAGS);
	data = &dram_info->dram_frags[alloc_id - 1];
	data->alloc_id = cpu_to_le32(alloc_id);
	data->num_frags = cpu_to_le32(num_frags);
	data->buf_location = cpu_to_le32(IWL_FW_INI_LOCATION_DRAM_PATH);

	IWL_DEBUG_FW(fwrt, "WRT: DRAM buffer details alloc_id=%u, num_frags=%u\n",
		     cpu_to_le32(alloc_id), cpu_to_le32(num_frags));

	for (j = 0; j < num_frags; j++) {
		struct iwl_buf_alloc_frag *frag = &data->frags[j];
		struct iwl_dram_data *fw_mon_frag = &fw_mon->frags[fw_mon_idx++];

		frag->addr = cpu_to_le64(fw_mon_frag->physical);
		frag->size = cpu_to_le32(fw_mon_frag->size);
		IWL_DEBUG_FW(fwrt, "WRT: DRAM fragment details\n");
		IWL_DEBUG_FW(fwrt, "frag=%u, addr=0x%016llx, size=0x%x)\n",
			     j, cpu_to_le64(fw_mon_frag->physical),
			     cpu_to_le32(fw_mon_frag->size));
	}
	return 0;
}

static void iwl_dbg_tlv_update_drams(struct iwl_fw_runtime *fwrt)
{
	int ret, i, dram_alloc = 0;
	struct iwl_dram_info dram_info;
	struct iwl_dram_data *frags =
		&fwrt->trans->dbg.fw_mon_ini[IWL_FW_INI_ALLOCATION_ID_DBGC1].frags[0];

	if (!fw_has_capa(&fwrt->fw->ucode_capa,
			 IWL_UCODE_TLV_CAPA_DRAM_FRAG_SUPPORT))
		return;

	dram_info.first_word = cpu_to_le32(DRAM_INFO_FIRST_MAGIC_WORD);
	dram_info.second_word = cpu_to_le32(DRAM_INFO_SECOND_MAGIC_WORD);

	for (i = IWL_FW_INI_ALLOCATION_ID_DBGC1;
	     i <= IWL_FW_INI_ALLOCATION_ID_DBGC3; i++) {
		ret = iwl_dbg_tlv_update_dram(fwrt, i, &dram_info);
		if (!ret)
			dram_alloc++;
		else
			IWL_WARN(fwrt,
				 "WRT: Failed to set DRAM buffer for alloc id %d, ret=%d\n",
				 i, ret);
	}
	if (dram_alloc) {
		memcpy(frags->block, &dram_info, sizeof(dram_info));
		IWL_DEBUG_FW(fwrt, "block data after  %016x\n",
			     *((int *)fwrt->trans->dbg.fw_mon_ini[1].frags[0].block));
	}
}

static void iwl_dbg_tlv_send_hcmds(struct iwl_fw_runtime *fwrt,
				   struct list_head *hcmd_list)
{
	struct iwl_dbg_tlv_node *node;
	}
}

static void iwl_dbg_tlv_apply_config(struct iwl_fw_runtime *fwrt,
				     struct list_head *config_list)
{
	struct iwl_dbg_tlv_node *node;

	list_for_each_entry(node, config_list, list) {
		struct iwl_fw_ini_conf_set_tlv *config_list = (void *)node->tlv.data;
		u32 count, address, value;
		u32 len = (le32_to_cpu(node->tlv.length) - sizeof(*config_list)) / 8;
		u32 type = le32_to_cpu(config_list->set_type);
		u32 offset = le32_to_cpu(config_list->addr_offset);

		switch (type) {
		case IWL_FW_INI_CONFIG_SET_TYPE_DEVICE_PERIPHERY_MAC: {
			if (!iwl_trans_grab_nic_access(fwrt->trans)) {
				IWL_DEBUG_FW(fwrt, "WRT: failed to get nic access\n");
				IWL_DEBUG_FW(fwrt, "WRT: skipping MAC PERIPHERY config\n");
				continue;
			}
			IWL_DEBUG_FW(fwrt, "WRT:  MAC PERIPHERY config len: len %u\n", len);
			for (count = 0; count < len; count++) {
				address = le32_to_cpu(config_list->addr_val[count].address);
				value = le32_to_cpu(config_list->addr_val[count].value);
				iwl_trans_write_prph(fwrt->trans, address + offset, value);
			}
			iwl_trans_release_nic_access(fwrt->trans);
		break;
		}
		case IWL_FW_INI_CONFIG_SET_TYPE_DEVICE_MEMORY: {
			for (count = 0; count < len; count++) {
				address = le32_to_cpu(config_list->addr_val[count].address);
				value = le32_to_cpu(config_list->addr_val[count].value);
				iwl_trans_write_mem32(fwrt->trans, address + offset, value);
				IWL_DEBUG_FW(fwrt, "WRT: DEV_MEM: count %u, add: %u val: %u\n",
					     count, address, value);
			}
		break;
		}
		case IWL_FW_INI_CONFIG_SET_TYPE_CSR: {
			for (count = 0; count < len; count++) {
				address = le32_to_cpu(config_list->addr_val[count].address);
				value = le32_to_cpu(config_list->addr_val[count].value);
				iwl_write32(fwrt->trans, address + offset, value);
				IWL_DEBUG_FW(fwrt, "WRT: CSR: count %u, add: %u val: %u\n",
					     count, address, value);
			}
		break;
		}
		case IWL_FW_INI_CONFIG_SET_TYPE_DBGC_DRAM_ADDR: {
			struct iwl_dbgc1_info dram_info = {};
			struct iwl_dram_data *frags = &fwrt->trans->dbg.fw_mon_ini[1].frags[0];
			__le64 dram_base_addr = cpu_to_le64(frags->physical);
			__le32 dram_size = cpu_to_le32(frags->size);
			u64  dram_addr = le64_to_cpu(dram_base_addr);
			u32 ret;

			IWL_DEBUG_FW(fwrt, "WRT: dram_base_addr 0x%016llx, dram_size 0x%x\n",
				     dram_base_addr, dram_size);
			IWL_DEBUG_FW(fwrt, "WRT: config_list->addr_offset: %u\n",
				     le32_to_cpu(config_list->addr_offset));
			for (count = 0; count < len; count++) {
				address = le32_to_cpu(config_list->addr_val[count].address);
				dram_info.dbgc1_add_lsb =
					cpu_to_le32((dram_addr & 0x00000000FFFFFFFFULL) + 0x400);
				dram_info.dbgc1_add_msb =
					cpu_to_le32((dram_addr & 0xFFFFFFFF00000000ULL) >> 32);
				dram_info.dbgc1_size = cpu_to_le32(le32_to_cpu(dram_size) - 0x400);
				ret = iwl_trans_write_mem(fwrt->trans,
							  address + offset, &dram_info, 4);
				if (ret) {
					IWL_ERR(fwrt, "Failed to write dram_info to HW_SMEM\n");
					break;
				}
			}
			break;
		}
		case IWL_FW_INI_CONFIG_SET_TYPE_PERIPH_SCRATCH_HWM: {
			u32 debug_token_config =
				le32_to_cpu(config_list->addr_val[0].value);

			IWL_DEBUG_FW(fwrt, "WRT: Setting HWM debug token config: %u\n",
				     debug_token_config);
			fwrt->trans->dbg.ucode_preset = debug_token_config;
			break;
		}
		default:
			break;
		}
	}
}

static void iwl_dbg_tlv_periodic_trig_handler(struct timer_list *t)
{
	struct iwl_dbg_tlv_timer_node *timer_node =
		from_timer(timer_node, t, timer);
			&fwrt->trans->dbg.fw_mon_cfg[i];
		u32 dest = le32_to_cpu(fw_mon_cfg->buf_location);

		if (dest == IWL_FW_INI_LOCATION_INVALID) {
			failed_alloc |= BIT(i);
			continue;
		}

		if (*ini_dest == IWL_FW_INI_LOCATION_INVALID)
			*ini_dest = dest;

			&fwrt->trans->dbg.active_regions[i];
		u32 reg_type;

		if (!*active_reg) {
			fwrt->trans->dbg.unsupported_region_msk |= BIT(i);
			continue;
		}

		reg = (void *)(*active_reg)->data;
		reg_type = le32_to_cpu(reg->type);

			     union iwl_dbg_tlv_tp_data *tp_data,
			     bool sync)
{
	struct list_head *hcmd_list, *trig_list, *conf_list;

	if (!iwl_trans_dbg_ini_valid(fwrt->trans) ||
	    tp_id == IWL_FW_INI_TIME_POINT_INVALID ||
	    tp_id >= IWL_FW_INI_TIME_POINT_NUM)

	hcmd_list = &fwrt->trans->dbg.time_point[tp_id].hcmd_list;
	trig_list = &fwrt->trans->dbg.time_point[tp_id].active_trig_list;
	conf_list = &fwrt->trans->dbg.time_point[tp_id].config_list;

	switch (tp_id) {
	case IWL_FW_INI_TIME_POINT_EARLY:
		iwl_dbg_tlv_init_cfg(fwrt);
		iwl_dbg_tlv_apply_config(fwrt, conf_list);
		iwl_dbg_tlv_update_drams(fwrt);
		iwl_dbg_tlv_tp_trigger(fwrt, sync, trig_list, tp_data, NULL);
		break;
	case IWL_FW_INI_TIME_POINT_AFTER_ALIVE:
		iwl_dbg_tlv_apply_buffers(fwrt);
		iwl_dbg_tlv_send_hcmds(fwrt, hcmd_list);
		iwl_dbg_tlv_apply_config(fwrt, conf_list);
		iwl_dbg_tlv_tp_trigger(fwrt, sync, trig_list, tp_data, NULL);
		break;
	case IWL_FW_INI_TIME_POINT_PERIODIC:
		iwl_dbg_tlv_set_periodic_trigs(fwrt);
	case IWL_FW_INI_TIME_POINT_MISSED_BEACONS:
	case IWL_FW_INI_TIME_POINT_FW_DHC_NOTIFICATION:
		iwl_dbg_tlv_send_hcmds(fwrt, hcmd_list);
		iwl_dbg_tlv_apply_config(fwrt, conf_list);
		iwl_dbg_tlv_tp_trigger(fwrt, sync, trig_list, tp_data,
				       iwl_dbg_tlv_check_fw_pkt);
		break;
	default:
		iwl_dbg_tlv_send_hcmds(fwrt, hcmd_list);
		iwl_dbg_tlv_apply_config(fwrt, conf_list);
		iwl_dbg_tlv_tp_trigger(fwrt, sync, trig_list, tp_data, NULL);
		break;
	}
}