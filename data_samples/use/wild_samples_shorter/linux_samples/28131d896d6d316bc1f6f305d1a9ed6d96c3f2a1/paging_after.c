// SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause
/*
 * Copyright (C) 2012-2014, 2018-2019, 2021 Intel Corporation
 * Copyright (C) 2013-2015 Intel Mobile Communications GmbH
 * Copyright (C) 2016-2017 Intel Deutschland GmbH
 */
#include "iwl-drv.h"
	memcpy(page_address(fwrt->fw_paging_db[0].fw_paging_block),
	       image->sec[sec_idx].data,
	       image->sec[sec_idx].len);
	fwrt->fw_paging_db[0].fw_offs = image->sec[sec_idx].offset;
	dma_sync_single_for_device(fwrt->trans->dev,
				   fwrt->fw_paging_db[0].fw_paging_phys,
				   fwrt->fw_paging_db[0].fw_paging_size,
				   DMA_BIDIRECTIONAL);

		memcpy(page_address(block->fw_paging_block),
		       image->sec[sec_idx].data + offset, len);
		block->fw_offs = image->sec[sec_idx].offset + offset;
		dma_sync_single_for_device(fwrt->trans->dev,
					   block->fw_paging_phys,
					   block->fw_paging_size,
					   DMA_BIDIRECTIONAL);