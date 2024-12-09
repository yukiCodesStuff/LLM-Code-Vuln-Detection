{
	struct tun_xdp_hdr *hdr = xdp->data_hard_start;
	struct virtio_net_hdr *gso = &hdr->gso;
	int buflen = hdr->buflen;
	int vnet_hdr_len = 0;
	struct tap_dev *tap;
	struct sk_buff *skb;
	int err, depth;

	if (unlikely(xdp->data_end - xdp->data < ETH_HLEN)) {
		err = -EINVAL;
		goto err;
	}