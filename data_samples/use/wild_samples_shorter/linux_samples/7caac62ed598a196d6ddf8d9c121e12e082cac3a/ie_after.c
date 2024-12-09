		}

		vs_ie = (struct ieee_types_header *)vendor_ie;
		if (le16_to_cpu(ie->ie_length) + vs_ie->len + 2 >
			IEEE_MAX_IE_SIZE)
			return -EINVAL;
		memcpy(ie->ie_buffer + le16_to_cpu(ie->ie_length),
		       vs_ie, vs_ie->len + 2);
		le16_unaligned_add_cpu(&ie->ie_length, vs_ie->len + 2);
		ie->mgmt_subtype_mask = cpu_to_le16(mask);