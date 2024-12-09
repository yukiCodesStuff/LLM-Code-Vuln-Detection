	unsigned char h_dest[ETH_ALEN];	/* destination eth addr	*/

	struct net_device *out_dev;
	struct net_device *route_dev;
	int tunnel_type;
	int tunnel_hlen;
	int reformat_type;
	u8 flags;