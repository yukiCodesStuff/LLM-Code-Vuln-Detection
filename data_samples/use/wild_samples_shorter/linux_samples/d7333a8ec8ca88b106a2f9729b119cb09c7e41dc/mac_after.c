		if (ar->bus_param.dev_type == ATH10K_DEV_TYPE_HL) {
			arvif->beacon_buf = kmalloc(IEEE80211_MAX_FRAME_LEN,
						    GFP_KERNEL);

			/* Using a kernel pointer in place of a dma_addr_t
			 * token can lead to undefined behavior if that
			 * makes it into cache management functions. Use a
			 * known-invalid address token instead, which
			 * avoids the warning and makes it easier to catch
			 * bugs if it does end up getting used.
			 */
			arvif->beacon_paddr = DMA_MAPPING_ERROR;
		} else {
			arvif->beacon_buf =
				dma_alloc_coherent(ar->dev,
						   IEEE80211_MAX_FRAME_LEN,