	mss_now -= icsk->icsk_ext_hdr_len;

	/* Then reserve room for full set of TCP options and 8 bytes of data */
	if (mss_now < TCP_MIN_SND_MSS)
		mss_now = TCP_MIN_SND_MSS;
	return mss_now;
}

/* Calculate MSS. Not accounting for SACKs here.  */