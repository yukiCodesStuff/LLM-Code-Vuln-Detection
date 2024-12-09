				macaddr = cam_const_broad;
				entry_id = key_index;
			} else {
				key_index = PAIRWISE_KEYIDX;
				entry_id = CAM_PAIRWISE_KEY_POSITION;
				is_pairwise = true;
			}
		}
		if (rtlpriv->sec.key_len[key_index] == 0) {
			RT_TRACE(rtlpriv, COMP_SEC, DBG_DMESG,
				 "delete one entry\n");
			rtl_cam_delete_one_entry(hw, p_macaddr, entry_id);
		} else {
			RT_TRACE(rtlpriv, COMP_SEC, DBG_LOUD,
				 "The insert KEY length is %d\n",