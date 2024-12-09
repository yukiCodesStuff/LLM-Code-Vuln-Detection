	info->sctpi_ictrlchunks = asoc->stats.ictrlchunks;

	prim = asoc->peer.primary_path;
	memcpy(&info->sctpi_p_address, &prim->ipaddr,
	       sizeof(struct sockaddr_storage));
	info->sctpi_p_state = prim->state;
	info->sctpi_p_cwnd = prim->cwnd;
	info->sctpi_p_srtt = prim->srtt;
	info->sctpi_p_rto = jiffies_to_msecs(prim->rto);