		if (ar->bus_param.dev_type == ATH10K_DEV_TYPE_HL) {
			arvif->beacon_buf = kmalloc(IEEE80211_MAX_FRAME_LEN,
						    GFP_KERNEL);
			arvif->beacon_paddr = (dma_addr_t)arvif->beacon_buf;
		} else {
			arvif->beacon_buf =
				dma_alloc_coherent(ar->dev,
						   IEEE80211_MAX_FRAME_LEN,
						   &arvif->beacon_paddr,
						   GFP_ATOMIC);
		}