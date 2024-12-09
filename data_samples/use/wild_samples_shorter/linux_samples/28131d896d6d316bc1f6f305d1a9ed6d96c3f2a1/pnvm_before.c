		goto out;
	}

	IWL_INFO(trans, "loaded PNVM version 0x%0x\n", sha1);

	ret = iwl_trans_set_pnvm(trans, pnvm_data, size);
out:
	kfree(pnvm_data);