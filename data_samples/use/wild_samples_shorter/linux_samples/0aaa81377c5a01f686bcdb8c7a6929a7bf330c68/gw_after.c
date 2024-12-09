	while (modidx < MAX_MODFUNCTIONS && gwj->mod.modfunc[modidx])
		(*gwj->mod.modfunc[modidx++])(cf, &gwj->mod);

	/* Has the CAN frame been modified? */
	if (modidx) {
		/* get available space for the processed CAN frame type */
		int max_len = nskb->len - offsetof(struct can_frame, data);

		/* dlc may have changed, make sure it fits to the CAN frame */
		if (cf->can_dlc > max_len)
			goto out_delete;

		/* check for checksum updates in classic CAN length only */
		if (gwj->mod.csumfunc.crc8) {
			if (cf->can_dlc > 8)
				goto out_delete;

			(*gwj->mod.csumfunc.crc8)(cf, &gwj->mod.csum.crc8);
		}

		if (gwj->mod.csumfunc.xor) {
			if (cf->can_dlc > 8)
				goto out_delete;

			(*gwj->mod.csumfunc.xor)(cf, &gwj->mod.csum.xor);
		}
	}

	/* clear the skb timestamp if not configured the other way */
	if (!(gwj->flags & CGW_FLAGS_CAN_SRC_TSTAMP))
		gwj->dropped_frames++;
	else
		gwj->handled_frames++;

	return;

 out_delete:
	/* delete frame due to misconfiguration */
	gwj->deleted_frames++;
	kfree_skb(nskb);
	return;
}

static inline int cgw_register_filter(struct net *net, struct cgw_job *gwj)
{