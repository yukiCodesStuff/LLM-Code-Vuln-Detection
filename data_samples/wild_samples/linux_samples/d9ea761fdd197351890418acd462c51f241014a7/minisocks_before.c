	if (newsk != NULL) {
		struct dccp_request_sock *dreq = dccp_rsk(req);
		struct inet_connection_sock *newicsk = inet_csk(newsk);
		struct dccp_sock *newdp = dccp_sk(newsk);

		newdp->dccps_role	    = DCCP_ROLE_SERVER;
		newdp->dccps_hc_rx_ackvec   = NULL;
		newdp->dccps_service_list   = NULL;
		newdp->dccps_service	    = dreq->dreq_service;
		newdp->dccps_timestamp_echo = dreq->dreq_timestamp_echo;
		newdp->dccps_timestamp_time = dreq->dreq_timestamp_time;
		newicsk->icsk_rto	    = DCCP_TIMEOUT_INIT;

		INIT_LIST_HEAD(&newdp->dccps_featneg);
		/*
		 * Step 3: Process LISTEN state
		 *
		 *    Choose S.ISS (initial seqno) or set from Init Cookies
		 *    Initialize S.GAR := S.ISS
		 *    Set S.ISR, S.GSR from packet (or Init Cookies)
		 *
		 *    Setting AWL/AWH and SWL/SWH happens as part of the feature
		 *    activation below, as these windows all depend on the local
		 *    and remote Sequence Window feature values (7.5.2).
		 */
		newdp->dccps_iss = dreq->dreq_iss;
		newdp->dccps_gss = dreq->dreq_gss;
		newdp->dccps_gar = newdp->dccps_iss;
		newdp->dccps_isr = dreq->dreq_isr;
		newdp->dccps_gsr = dreq->dreq_gsr;

		/*
		 * Activate features: initialise CCIDs, sequence windows etc.
		 */
		if (dccp_feat_activate_values(newsk, &dreq->dreq_featneg)) {
			sk_free_unlock_clone(newsk);
			return NULL;
		}