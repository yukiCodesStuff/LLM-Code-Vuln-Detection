						  struct flowi *fl,
						  int reverse);
	int			(*get_tos)(const struct flowi *fl);
	void			(*init_dst)(struct net *net,
					    struct xfrm_dst *dst);
	int			(*init_path)(struct xfrm_dst *path,
					     struct dst_entry *dst,
					     int nfheader_len);
	int			(*fill_dst)(struct xfrm_dst *xdst,