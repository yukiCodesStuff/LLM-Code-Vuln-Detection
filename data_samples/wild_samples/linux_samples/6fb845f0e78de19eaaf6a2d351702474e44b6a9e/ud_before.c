	    opcode == IB_OPCODE_UD_SEND_ONLY_WITH_IMMEDIATE) {
		wc.ex.imm_data = packet->ohdr->u.ud.imm_data;
		wc.wc_flags = IB_WC_WITH_IMM;
		tlen -= sizeof(u32);
	} else if (opcode == IB_OPCODE_UD_SEND_ONLY) {
		wc.ex.imm_data = 0;
		wc.wc_flags = 0;
	} else {
		goto drop;
	}

	/*
	 * A GRH is expected to precede the data even if not
	 * present on the wire.
	 */
	wc.byte_len = tlen + sizeof(struct ib_grh);

	/*
	 * Get the next work request entry to find where to put the data.
	 */
	if (qp->r_flags & RVT_R_REUSE_SGE) {
		qp->r_flags &= ~RVT_R_REUSE_SGE;
	} else {
		int ret;

		ret = rvt_get_rwqe(qp, false);
		if (ret < 0) {
			rvt_rc_error(qp, IB_WC_LOC_QP_OP_ERR);
			return;
		}
		if (!ret) {
			if (qp->ibqp.qp_num == 0)
				ibp->rvp.n_vl15_dropped++;
			return;
		}
	}
	/* Silently drop packets which are too big. */
	if (unlikely(wc.byte_len > qp->r_len)) {
		qp->r_flags |= RVT_R_REUSE_SGE;
		goto drop;
	}
	if (packet->grh) {
		rvt_copy_sge(qp, &qp->r_sge, packet->grh,
			     sizeof(struct ib_grh), true, false);
		wc.wc_flags |= IB_WC_GRH;
	} else if (packet->etype == RHF_RCV_TYPE_BYPASS) {
		struct ib_grh grh;
		/*
		 * Assuming we only created 16B on the send side
		 * if we want to use large LIDs, since GRH was stripped
		 * out when creating 16B, add back the GRH here.
		 */
		hfi1_make_ext_grh(packet, &grh, slid, dlid);
		rvt_copy_sge(qp, &qp->r_sge, &grh,
			     sizeof(struct ib_grh), true, false);
		wc.wc_flags |= IB_WC_GRH;
	} else {
		rvt_skip_sge(&qp->r_sge, sizeof(struct ib_grh), true);
	}
	rvt_copy_sge(qp, &qp->r_sge, data, wc.byte_len - sizeof(struct ib_grh),
		     true, false);
	rvt_put_ss(&qp->r_sge);
	if (!test_and_clear_bit(RVT_R_WRID_VALID, &qp->r_aflags))
		return;
	wc.wr_id = qp->r_wr_id;
	wc.status = IB_WC_SUCCESS;
	wc.opcode = IB_WC_RECV;
	wc.vendor_err = 0;
	wc.qp = &qp->ibqp;
	wc.src_qp = src_qp;

	if (qp->ibqp.qp_type == IB_QPT_GSI ||
	    qp->ibqp.qp_type == IB_QPT_SMI) {
		if (mgmt_pkey_idx < 0) {
			if (net_ratelimit()) {
				struct hfi1_devdata *dd = ppd->dd;

				dd_dev_err(dd, "QP type %d mgmt_pkey_idx < 0 and packet not dropped???\n",
					   qp->ibqp.qp_type);
				mgmt_pkey_idx = 0;
			}
		}
		wc.pkey_index = (unsigned)mgmt_pkey_idx;
	} else {
		wc.pkey_index = 0;
	}
	if (slid_is_permissive)
		slid = be32_to_cpu(OPA_LID_PERMISSIVE);
	wc.slid = slid & U16_MAX;
	wc.sl = sl_from_sc;

	/*
	 * Save the LMC lower bits if the destination LID is a unicast LID.
	 */
	wc.dlid_path_bits = hfi1_check_mcast(dlid) ? 0 :
		dlid & ((1 << ppd_from_ibp(ibp)->lmc) - 1);
	wc.port_num = qp->port_num;
	/* Signal completion event if the solicited bit is set. */
	rvt_cq_enter(ibcq_to_rvtcq(qp->ibqp.recv_cq), &wc, solicited);
	return;

drop:
	ibp->rvp.n_pkt_drops++;
}