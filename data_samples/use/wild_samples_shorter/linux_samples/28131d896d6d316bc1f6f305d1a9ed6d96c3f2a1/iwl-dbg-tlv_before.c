 * @IWL_DBG_TLV_TYPE_HCMD: host command TLV
 * @IWL_DBG_TLV_TYPE_REGION: region TLV
 * @IWL_DBG_TLV_TYPE_TRIGGER: trigger TLV
 * @IWL_DBG_TLV_TYPE_NUM: number of debug TLVs
 */
enum iwl_dbg_tlv_type {
	IWL_DBG_TLV_TYPE_DEBUG_INFO =
	IWL_DBG_TLV_TYPE_HCMD,
	IWL_DBG_TLV_TYPE_REGION,
	IWL_DBG_TLV_TYPE_TRIGGER,
	IWL_DBG_TLV_TYPE_NUM,
};

/**
	[IWL_DBG_TLV_TYPE_HCMD]		= {.min_ver = 1, .max_ver = 1,},
	[IWL_DBG_TLV_TYPE_REGION]	= {.min_ver = 1, .max_ver = 2,},
	[IWL_DBG_TLV_TYPE_TRIGGER]	= {.min_ver = 1, .max_ver = 1,},
};

static int iwl_dbg_tlv_add(const struct iwl_ucode_tlv *tlv,
			   struct list_head *list)
	return ret;
}

static int (*dbg_tlv_alloc[])(struct iwl_trans *trans,
			      const struct iwl_ucode_tlv *tlv) = {
	[IWL_DBG_TLV_TYPE_DEBUG_INFO]	= iwl_dbg_tlv_alloc_debug_info,
	[IWL_DBG_TLV_TYPE_BUF_ALLOC]	= iwl_dbg_tlv_alloc_buf_alloc,
	[IWL_DBG_TLV_TYPE_HCMD]		= iwl_dbg_tlv_alloc_hcmd,
	[IWL_DBG_TLV_TYPE_REGION]	= iwl_dbg_tlv_alloc_region,
	[IWL_DBG_TLV_TYPE_TRIGGER]	= iwl_dbg_tlv_alloc_trigger,
};

void iwl_dbg_tlv_alloc(struct iwl_trans *trans, const struct iwl_ucode_tlv *tlv,
		       bool ext)
			list_del(&tlv_node->list);
			kfree(tlv_node);
		}
	}

	for (i = 0; i < ARRAY_SIZE(trans->dbg.fw_mon_ini); i++)
		iwl_dbg_tlv_fragments_free(trans, i);
		INIT_LIST_HEAD(&tp->trig_list);
		INIT_LIST_HEAD(&tp->hcmd_list);
		INIT_LIST_HEAD(&tp->active_trig_list);
	}
}

static int iwl_dbg_tlv_alloc_fragment(struct iwl_fw_runtime *fwrt,
{
	int ret, i;

	for (i = 0; i < IWL_FW_INI_ALLOCATION_NUM; i++) {
		ret = iwl_dbg_tlv_apply_buffer(fwrt, i);
		if (ret)
			IWL_WARN(fwrt,
	}
}

static void iwl_dbg_tlv_send_hcmds(struct iwl_fw_runtime *fwrt,
				   struct list_head *hcmd_list)
{
	struct iwl_dbg_tlv_node *node;
	}
}

static void iwl_dbg_tlv_periodic_trig_handler(struct timer_list *t)
{
	struct iwl_dbg_tlv_timer_node *timer_node =
		from_timer(timer_node, t, timer);
			&fwrt->trans->dbg.fw_mon_cfg[i];
		u32 dest = le32_to_cpu(fw_mon_cfg->buf_location);

		if (dest == IWL_FW_INI_LOCATION_INVALID)
			continue;

		if (*ini_dest == IWL_FW_INI_LOCATION_INVALID)
			*ini_dest = dest;

			&fwrt->trans->dbg.active_regions[i];
		u32 reg_type;

		if (!*active_reg)
			continue;

		reg = (void *)(*active_reg)->data;
		reg_type = le32_to_cpu(reg->type);

			     union iwl_dbg_tlv_tp_data *tp_data,
			     bool sync)
{
	struct list_head *hcmd_list, *trig_list;

	if (!iwl_trans_dbg_ini_valid(fwrt->trans) ||
	    tp_id == IWL_FW_INI_TIME_POINT_INVALID ||
	    tp_id >= IWL_FW_INI_TIME_POINT_NUM)

	hcmd_list = &fwrt->trans->dbg.time_point[tp_id].hcmd_list;
	trig_list = &fwrt->trans->dbg.time_point[tp_id].active_trig_list;

	switch (tp_id) {
	case IWL_FW_INI_TIME_POINT_EARLY:
		iwl_dbg_tlv_init_cfg(fwrt);
		iwl_dbg_tlv_tp_trigger(fwrt, sync, trig_list, tp_data, NULL);
		break;
	case IWL_FW_INI_TIME_POINT_AFTER_ALIVE:
		iwl_dbg_tlv_apply_buffers(fwrt);
		iwl_dbg_tlv_send_hcmds(fwrt, hcmd_list);
		iwl_dbg_tlv_tp_trigger(fwrt, sync, trig_list, tp_data, NULL);
		break;
	case IWL_FW_INI_TIME_POINT_PERIODIC:
		iwl_dbg_tlv_set_periodic_trigs(fwrt);
	case IWL_FW_INI_TIME_POINT_MISSED_BEACONS:
	case IWL_FW_INI_TIME_POINT_FW_DHC_NOTIFICATION:
		iwl_dbg_tlv_send_hcmds(fwrt, hcmd_list);
		iwl_dbg_tlv_tp_trigger(fwrt, sync, trig_list, tp_data,
				       iwl_dbg_tlv_check_fw_pkt);
		break;
	default:
		iwl_dbg_tlv_send_hcmds(fwrt, hcmd_list);
		iwl_dbg_tlv_tp_trigger(fwrt, sync, trig_list, tp_data, NULL);
		break;
	}
}