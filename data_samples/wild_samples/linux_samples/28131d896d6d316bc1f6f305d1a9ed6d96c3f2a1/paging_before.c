// SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause
/*
 * Copyright (C) 2012-2014, 2018-2019 Intel Corporation
 * Copyright (C) 2013-2015 Intel Mobile Communications GmbH
 * Copyright (C) 2016-2017 Intel Deutschland GmbH
 */
#include "iwl-drv.h"
#include "runtime.h"
#include "fw/api/commands.h"

void iwl_free_fw_paging(struct iwl_fw_runtime *fwrt)
{
	int i;

	if (!fwrt->fw_paging_db[0].fw_paging_block)
		return;

	for (i = 0; i < NUM_OF_FW_PAGING_BLOCKS; i++) {
		struct iwl_fw_paging *paging = &fwrt->fw_paging_db[i];

		if (!paging->fw_paging_block) {
			IWL_DEBUG_FW(fwrt,
				     "Paging: block %d already freed, continue to next page\n",
				     i);

			continue;
		}
		dma_unmap_page(fwrt->trans->dev, paging->fw_paging_phys,
			       paging->fw_paging_size, DMA_BIDIRECTIONAL);

		__free_pages(paging->fw_paging_block,
			     get_order(paging->fw_paging_size));
		paging->fw_paging_block = NULL;
	}

	memset(fwrt->fw_paging_db, 0, sizeof(fwrt->fw_paging_db));
}
	if (image->sec[sec_idx].len > fwrt->fw_paging_db[0].fw_paging_size) {
		IWL_ERR(fwrt, "CSS block is larger than paging size\n");
		ret = -EINVAL;
		goto err;
	}

	memcpy(page_address(fwrt->fw_paging_db[0].fw_paging_block),
	       image->sec[sec_idx].data,
	       image->sec[sec_idx].len);
	dma_sync_single_for_device(fwrt->trans->dev,
				   fwrt->fw_paging_db[0].fw_paging_phys,
				   fwrt->fw_paging_db[0].fw_paging_size,
				   DMA_BIDIRECTIONAL);

	IWL_DEBUG_FW(fwrt,
		     "Paging: copied %d CSS bytes to first block\n",
		     fwrt->fw_paging_db[0].fw_paging_size);

	sec_idx++;

	/*
	 * Copy the paging blocks to the dram.  The loop index starts
	 * from 1 since the CSS block (index 0) was already copied to
	 * dram.  We use num_of_paging_blk + 1 to account for that.
	 */
	for (idx = 1; idx < fwrt->num_of_paging_blk + 1; idx++) {
		struct iwl_fw_paging *block = &fwrt->fw_paging_db[idx];
		int remaining = image->sec[sec_idx].len - offset;
		int len = block->fw_paging_size;

		/*
		 * For the last block, we copy all that is remaining,
		 * for all other blocks, we copy fw_paging_size at a
		 * time. */
		if (idx == fwrt->num_of_paging_blk) {
			len = remaining;
			if (remaining !=
			    fwrt->num_of_pages_in_last_blk * FW_PAGING_SIZE) {
				IWL_ERR(fwrt,
					"Paging: last block contains more data than expected %d\n",
					remaining);
				ret = -EINVAL;
				goto err;
			}
		} else if (block->fw_paging_size > remaining) {
			IWL_ERR(fwrt,
				"Paging: not enough data in other in block %d (%d)\n",
				idx, remaining);
			ret = -EINVAL;
			goto err;
		}

		memcpy(page_address(block->fw_paging_block),
		       image->sec[sec_idx].data + offset, len);
		dma_sync_single_for_device(fwrt->trans->dev,
					   block->fw_paging_phys,
					   block->fw_paging_size,
					   DMA_BIDIRECTIONAL);

		IWL_DEBUG_FW(fwrt,
			     "Paging: copied %d paging bytes to block %d\n",
			     len, idx);

		offset += block->fw_paging_size;
	}

	return 0;

err:
	iwl_free_fw_paging(fwrt);
	return ret;
}
		} else if (block->fw_paging_size > remaining) {
			IWL_ERR(fwrt,
				"Paging: not enough data in other in block %d (%d)\n",
				idx, remaining);
			ret = -EINVAL;
			goto err;
		}

		memcpy(page_address(block->fw_paging_block),
		       image->sec[sec_idx].data + offset, len);
		dma_sync_single_for_device(fwrt->trans->dev,
					   block->fw_paging_phys,
					   block->fw_paging_size,
					   DMA_BIDIRECTIONAL);

		IWL_DEBUG_FW(fwrt,
			     "Paging: copied %d paging bytes to block %d\n",
			     len, idx);

		offset += block->fw_paging_size;
	}

	return 0;

err:
	iwl_free_fw_paging(fwrt);
	return ret;
}

static int iwl_save_fw_paging(struct iwl_fw_runtime *fwrt,
			      const struct fw_img *fw)
{
	int ret;

	ret = iwl_alloc_fw_paging_mem(fwrt, fw);
	if (ret)
		return ret;

	return iwl_fill_paging_mem(fwrt, fw);
}

/* send paging cmd to FW in case CPU2 has paging image */
static int iwl_send_paging_cmd(struct iwl_fw_runtime *fwrt,
			       const struct fw_img *fw)
{
	struct iwl_fw_paging_cmd paging_cmd = {
		.flags = cpu_to_le32(PAGING_CMD_IS_SECURED |
				     PAGING_CMD_IS_ENABLED |
				     (fwrt->num_of_pages_in_last_blk <<
				      PAGING_CMD_NUM_OF_PAGES_IN_LAST_GRP_POS)),
		.block_size = cpu_to_le32(BLOCK_2_EXP_SIZE),
		.block_num = cpu_to_le32(fwrt->num_of_paging_blk),
	};
	struct iwl_host_cmd hcmd = {
		.id = iwl_cmd_id(FW_PAGING_BLOCK_CMD, IWL_ALWAYS_LONG_GROUP, 0),
		.len = { sizeof(paging_cmd), },
		.data = { &paging_cmd, },
	};
	int blk_idx;

	/* loop for for all paging blocks + CSS block */
	for (blk_idx = 0; blk_idx < fwrt->num_of_paging_blk + 1; blk_idx++) {
		dma_addr_t addr = fwrt->fw_paging_db[blk_idx].fw_paging_phys;
		__le32 phy_addr;

		addr = addr >> PAGE_2_EXP_SIZE;
		phy_addr = cpu_to_le32(addr);
		paging_cmd.device_phy_addr[blk_idx] = phy_addr;
	}

	return iwl_trans_send_cmd(fwrt->trans, &hcmd);
}