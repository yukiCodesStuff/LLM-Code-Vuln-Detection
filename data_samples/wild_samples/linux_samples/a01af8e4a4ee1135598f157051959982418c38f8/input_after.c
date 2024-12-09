	     between48(ackno, lawl, dp->dccps_awh))) {
		dccp_update_gsr(sk, seqno);

		if (dh->dccph_type != DCCP_PKT_SYNC &&
		    ackno != DCCP_PKT_WITHOUT_ACK_SEQ &&
		    after48(ackno, dp->dccps_gar))
			dp->dccps_gar = ackno;
	} else {
		unsigned long now = jiffies;
		/*
		 *   Step 6: Check sequence numbers
		 *      Otherwise,
		 *         If P.type == Reset,
		 *            Send Sync packet acknowledging S.GSR
		 *         Otherwise,
		 *            Send Sync packet acknowledging P.seqno
		 *      Drop packet and return
		 *
		 *   These Syncs are rate-limited as per RFC 4340, 7.5.4:
		 *   at most 1 / (dccp_sync_rate_limit * HZ) Syncs per second.
		 */
		if (time_before(now, (dp->dccps_rate_last +
				      sysctl_dccp_sync_ratelimit)))
			return 0;

		DCCP_WARN("Step 6 failed for %s packet, "
			  "(LSWL(%llu) <= P.seqno(%llu) <= S.SWH(%llu)) and "
			  "(P.ackno %s or LAWL(%llu) <= P.ackno(%llu) <= S.AWH(%llu), "
			  "sending SYNC...\n",  dccp_packet_name(dh->dccph_type),
			  (unsigned long long) lswl, (unsigned long long) seqno,
			  (unsigned long long) dp->dccps_swh,
			  (ackno == DCCP_PKT_WITHOUT_ACK_SEQ) ? "doesn't exist"
							      : "exists",
			  (unsigned long long) lawl, (unsigned long long) ackno,
			  (unsigned long long) dp->dccps_awh);

		dp->dccps_rate_last = now;

		if (dh->dccph_type == DCCP_PKT_RESET)
			seqno = dp->dccps_gsr;
		dccp_send_sync(sk, seqno, DCCP_PKT_SYNC);
		return -1;
	}