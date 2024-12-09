	switch (optname) {
	case DCCP_SOCKOPT_CCID_TX_INFO:
		if (len < sizeof(tfrc))
			return -EINVAL;
		memset(&tfrc, 0, sizeof(tfrc));
		tfrc.tfrctx_x	   = hc->tx_x;
		tfrc.tfrctx_x_recv = hc->tx_x_recv;
		tfrc.tfrctx_x_calc = hc->tx_x_calc;
		tfrc.tfrctx_rtt	   = hc->tx_rtt;
		tfrc.tfrctx_p	   = hc->tx_p;
		tfrc.tfrctx_rto	   = hc->tx_t_rto;
		tfrc.tfrctx_ipi	   = hc->tx_t_ipi;
		len = sizeof(tfrc);
		val = &tfrc;
		break;
	default:
		return -ENOPROTOOPT;
	}

	if (put_user(len, optlen) || copy_to_user(optval, val, len))
		return -EFAULT;

	return 0;
}

/*
 *	Receiver Half-Connection Routines
 */

/* CCID3 feedback types */
enum ccid3_fback_type {
	CCID3_FBACK_NONE = 0,
	CCID3_FBACK_INITIAL,
	CCID3_FBACK_PERIODIC,
	CCID3_FBACK_PARAM_CHANGE
};

#ifdef CONFIG_IP_DCCP_CCID3_DEBUG
static const char *ccid3_rx_state_name(enum ccid3_hc_rx_states state)
{
	static const char *const ccid3_rx_state_names[] = {
	[TFRC_RSTATE_NO_DATA] = "NO_DATA",
	[TFRC_RSTATE_DATA]    = "DATA",
	};

	return ccid3_rx_state_names[state];
}