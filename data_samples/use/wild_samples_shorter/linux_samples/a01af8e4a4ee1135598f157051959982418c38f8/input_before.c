		dccp_update_gsr(sk, seqno);

		if (dh->dccph_type != DCCP_PKT_SYNC &&
		    (ackno != DCCP_PKT_WITHOUT_ACK_SEQ))
			dp->dccps_gar = ackno;
	} else {
		unsigned long now = jiffies;
		/*