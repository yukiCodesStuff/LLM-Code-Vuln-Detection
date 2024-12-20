	struct brcmf_cfg80211_info *cfg = ifp->drvr->config;
	s32 status;
	struct brcmf_escan_result_le *escan_result_le;
	u32 escan_buflen;
	struct brcmf_bss_info_le *bss_info_le;
	struct brcmf_bss_info_le *bss = NULL;
	u32 bi_length;
	struct brcmf_scan_results *list;

	if (status == BRCMF_E_STATUS_PARTIAL) {
		brcmf_dbg(SCAN, "ESCAN Partial result\n");
		if (e->datalen < sizeof(*escan_result_le)) {
			brcmf_err("invalid event data length\n");
			goto exit;
		}
		escan_result_le = (struct brcmf_escan_result_le *) data;
		if (!escan_result_le) {
			brcmf_err("Invalid escan result (NULL pointer)\n");
			goto exit;
		}
		escan_buflen = le32_to_cpu(escan_result_le->buflen);
		if (escan_buflen > BRCMF_ESCAN_BUF_SIZE ||
		    escan_buflen > e->datalen ||
		    escan_buflen < sizeof(*escan_result_le)) {
			brcmf_err("Invalid escan buffer length: %d\n",
				  escan_buflen);
			goto exit;
		}
		if (le16_to_cpu(escan_result_le->bss_count) != 1) {
			brcmf_err("Invalid bss_count %d: ignoring\n",
				  escan_result_le->bss_count);
			goto exit;
		}

		bi_length = le32_to_cpu(bss_info_le->length);
		if (bi_length != escan_buflen -	WL_ESCAN_RESULTS_FIXED_SIZE) {
			brcmf_err("Ignoring invalid bss_info length: %d\n",
				  bi_length);
			goto exit;
		}
