	/* Decode and fetch the destination operand: register or memory. */
	rc = decode_operand(ctxt, &ctxt->dst, (ctxt->d >> DstShift) & OpMask);

	if (ctxt->rip_relative)
		ctxt->memopp->addr.mem.ea = address_mask(ctxt,
					ctxt->memopp->addr.mem.ea + ctxt->_eip);

done: