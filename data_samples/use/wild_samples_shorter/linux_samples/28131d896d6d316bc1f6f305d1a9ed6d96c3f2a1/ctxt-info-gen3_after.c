			dbg_flags |= IWL_PRPH_SCRATCH_EDBG_DEST_DRAM;
			dbg_cfg->hwm_base_addr = cpu_to_le64(frag->physical);
			dbg_cfg->hwm_size = cpu_to_le32(frag->size);
			dbg_cfg->debug_token_config = cpu_to_le32(trans->dbg.ucode_preset);
			IWL_DEBUG_FW(trans,
				     "WRT: Applying DRAM destination (debug_token_config=%u)\n",
				     dbg_cfg->debug_token_config);
			IWL_DEBUG_FW(trans,
				     "WRT: Applying DRAM destination (alloc_id=%u, num_frags=%u)\n",
				     alloc_id,
				     trans->dbg.fw_mon_ini[alloc_id].num_frags);