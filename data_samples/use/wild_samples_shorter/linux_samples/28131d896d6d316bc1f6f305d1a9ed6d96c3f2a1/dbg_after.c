	iwl_trans_read_prph(fwrt->trans, TXF_READ_MODIFY_DATA + offset);

	/* Read FIFO */
	for (i = 0; i < fifo_len / sizeof(u32); i++)
		fifo_data[i] = iwl_trans_read_prph(fwrt->trans,
						  TXF_READ_MODIFY_DATA +
						  offset);

	if (fwrt->sanitize_ops && fwrt->sanitize_ops->frob_txf)
		fwrt->sanitize_ops->frob_txf(fwrt->sanitize_ctx,
					     fifo_data, fifo_len);

	*dump_data = iwl_fw_error_next_data(*dump_data);
}

static void iwl_fw_dump_rxf(struct iwl_fw_runtime *fwrt,
	iwl_trans_read_mem_bytes(fwrt->trans, ofs, dump_mem->data, len);
	*dump_data = iwl_fw_error_next_data(*dump_data);

	if (fwrt->sanitize_ops && fwrt->sanitize_ops->frob_mem)
		fwrt->sanitize_ops->frob_mem(fwrt->sanitize_ctx, ofs,
					     dump_mem->data, len);

	IWL_DEBUG_INFO(fwrt, "WRT memory dump. Type=%u\n", dump_mem->type);
}

#define ADD_LEN(len, item_len, const_len) \
					   PAGING_BLOCK_SIZE,
					   DMA_BIDIRECTIONAL);
		(*data) = iwl_fw_error_next_data(*data);

		if (fwrt->sanitize_ops && fwrt->sanitize_ops->frob_mem)
			fwrt->sanitize_ops->frob_mem(fwrt->sanitize_ctx,
						     fwrt->fw_paging_db[i].fw_offs,
						     paging->data,
						     PAGING_BLOCK_SIZE);
	}
}

static struct iwl_fw_error_dump_file *
					 dump_data->data + data_size,
					 data_size);

		if (fwrt->sanitize_ops && fwrt->sanitize_ops->frob_mem)
			fwrt->sanitize_ops->frob_mem(fwrt->sanitize_ctx, addr,
						     dump_data->data + data_size,
						     data_size);

		dump_data = iwl_fw_error_next_data(dump_data);
	}

	/* Dump fw's virtual image */
	iwl_trans_read_mem_bytes(fwrt->trans, addr, range->data,
				 le32_to_cpu(reg->dev_addr.size));

	if ((le32_to_cpu(reg->id) & IWL_FW_INI_REGION_V2_MASK) ==
		IWL_FW_INI_HW_SMEM_REGION_ID &&
	    fwrt->sanitize_ops && fwrt->sanitize_ops->frob_txf)
		fwrt->sanitize_ops->frob_txf(fwrt->sanitize_ctx,
					     range->data,
					     le32_to_cpu(reg->dev_addr.size));

	return sizeof(*range) + le32_to_cpu(range->range_data_size);
}

static int _iwl_dump_ini_paging_iter(struct iwl_fw_runtime *fwrt,
	for (i = 0; i < iter->fifo_size; i += sizeof(*data))
		*data++ = cpu_to_le32(iwl_read_prph_no_grab(fwrt->trans, addr));

	if (fwrt->sanitize_ops && fwrt->sanitize_ops->frob_txf)
		fwrt->sanitize_ops->frob_txf(fwrt->sanitize_ctx,
					     reg_dump, iter->fifo_size);

out:
	iwl_trans_release_nic_access(fwrt->trans);

	return sizeof(*range) + le32_to_cpu(range->range_data_size);
	 */
	hw_type = CSR_HW_REV_TYPE(fwrt->trans->hw_rev);
	if (hw_type == IWL_AX210_HW_TYPE) {
		u32 prph_val = iwl_read_prph(fwrt->trans, WFPM_OTP_CFG1_ADDR_GEN2);
		u32 is_jacket = !!(prph_val & WFPM_OTP_CFG1_IS_JACKET_BIT);
		u32 is_cdb = !!(prph_val & WFPM_OTP_CFG1_IS_CDB_BIT);
		u32 masked_bits = is_jacket | (is_cdb << 1);

	if (dump_data->monitor_only)
		dump_mask &= BIT(IWL_FW_ERROR_DUMP_FW_MONITOR);

	fw_error_dump.trans_ptr = iwl_trans_dump_data(fwrt->trans, dump_mask,
						      fwrt->sanitize_ops,
						      fwrt->sanitize_ctx);
	file_len = le32_to_cpu(dump_file->file_len);
	fw_error_dump.fwrt_len = file_len;

	if (fw_error_dump.trans_ptr) {
	iwl_trans_read_mem_bytes(fwrt->trans, cfg->d3_debug_data_base_addr,
				 fwrt->dump.d3_debug_data,
				 cfg->d3_debug_data_length);

	if (fwrt->sanitize_ops && fwrt->sanitize_ops->frob_mem)
		fwrt->sanitize_ops->frob_mem(fwrt->sanitize_ctx,
					     cfg->d3_debug_data_base_addr,
					     fwrt->dump.d3_debug_data,
					     cfg->d3_debug_data_length);
}
IWL_EXPORT_SYMBOL(iwl_fw_dbg_read_d3_debug_data);

void iwl_fw_dbg_stop_sync(struct iwl_fw_runtime *fwrt)