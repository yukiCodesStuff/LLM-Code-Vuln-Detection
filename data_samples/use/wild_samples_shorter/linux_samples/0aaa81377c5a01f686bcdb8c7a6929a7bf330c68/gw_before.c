	while (modidx < MAX_MODFUNCTIONS && gwj->mod.modfunc[modidx])
		(*gwj->mod.modfunc[modidx++])(cf, &gwj->mod);

	/* check for checksum updates when the CAN frame has been modified */
	if (modidx) {
		if (gwj->mod.csumfunc.crc8)
			(*gwj->mod.csumfunc.crc8)(cf, &gwj->mod.csum.crc8);

		if (gwj->mod.csumfunc.xor)
			(*gwj->mod.csumfunc.xor)(cf, &gwj->mod.csum.xor);
	}

	/* clear the skb timestamp if not configured the other way */
	if (!(gwj->flags & CGW_FLAGS_CAN_SRC_TSTAMP))
		gwj->dropped_frames++;
	else
		gwj->handled_frames++;
}

static inline int cgw_register_filter(struct net *net, struct cgw_job *gwj)
{