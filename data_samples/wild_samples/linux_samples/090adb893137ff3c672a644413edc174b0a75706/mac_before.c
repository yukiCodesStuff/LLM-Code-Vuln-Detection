			if (is_group) {
				macaddr = cam_const_broad;
				entry_id = key_index;
			} else {
				key_index = PAIRWISE_KEYIDX;
				entry_id = CAM_PAIRWISE_KEY_POSITION;
				is_pairwise = true;
			}