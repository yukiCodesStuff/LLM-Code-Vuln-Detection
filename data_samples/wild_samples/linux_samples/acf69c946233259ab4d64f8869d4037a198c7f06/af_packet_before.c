{
	struct sock *sk;
	struct packet_sock *po;
	struct sockaddr_ll *sll;
	union tpacket_uhdr h;
	u8 *skb_head = skb->data;
	int skb_len = skb->len;
	unsigned int snaplen, res;
	unsigned long status = TP_STATUS_USER;
	unsigned short macoff, netoff, hdrlen;
	struct sk_buff *copy_skb = NULL;
	struct timespec64 ts;
	__u32 ts_status;
	bool is_drop_n_account = false;
	unsigned int slot_id = 0;
	bool do_vnet = false;

	/* struct tpacket{2,3}_hdr is aligned to a multiple of TPACKET_ALIGNMENT.
	 * We may add members to them until current aligned size without forcing
	 * userspace to call getsockopt(..., PACKET_HDRLEN, ...).
	 */
	BUILD_BUG_ON(TPACKET_ALIGN(sizeof(*h.h2)) != 32);
	BUILD_BUG_ON(TPACKET_ALIGN(sizeof(*h.h3)) != 48);

	if (skb->pkt_type == PACKET_LOOPBACK)
		goto drop;

	sk = pt->af_packet_priv;
	po = pkt_sk(sk);

	if (!net_eq(dev_net(dev), sock_net(sk)))
		goto drop;

	if (dev->header_ops) {
		if (sk->sk_type != SOCK_DGRAM)
			skb_push(skb, skb->data - skb_mac_header(skb));
		else if (skb->pkt_type == PACKET_OUTGOING) {
			/* Special case: outgoing packets have ll header at head */
			skb_pull(skb, skb_network_offset(skb));
		}
	}
		if (po->has_vnet_hdr) {
			netoff += sizeof(struct virtio_net_hdr);
			do_vnet = true;
		}
		macoff = netoff - maclen;
	}
	if (po->tp_version <= TPACKET_V2) {
		if (macoff + snaplen > po->rx_ring.frame_size) {
			if (po->copy_thresh &&
			    atomic_read(&sk->sk_rmem_alloc) < sk->sk_rcvbuf) {
				if (skb_shared(skb)) {
					copy_skb = skb_clone(skb, GFP_ATOMIC);
				} else {
					copy_skb = skb_get(skb);
					skb_head = skb->data;
				}
				if (copy_skb)
					skb_set_owner_r(copy_skb, sk);
			}
			snaplen = po->rx_ring.frame_size - macoff;
			if ((int)snaplen < 0) {
				snaplen = 0;
				do_vnet = false;
			}
		}
	} else if (unlikely(macoff + snaplen >
			    GET_PBDQC_FROM_RB(&po->rx_ring)->max_frame_len)) {
		u32 nval;

		nval = GET_PBDQC_FROM_RB(&po->rx_ring)->max_frame_len - macoff;
		pr_err_once("tpacket_rcv: packet too big, clamped from %u to %u. macoff=%u\n",
			    snaplen, nval, macoff);
		snaplen = nval;
		if (unlikely((int)snaplen < 0)) {
			snaplen = 0;
			macoff = GET_PBDQC_FROM_RB(&po->rx_ring)->max_frame_len;
			do_vnet = false;
		}
	}
	spin_lock(&sk->sk_receive_queue.lock);
	h.raw = packet_current_rx_frame(po, skb,
					TP_STATUS_KERNEL, (macoff+snaplen));
	if (!h.raw)
		goto drop_n_account;

	if (po->tp_version <= TPACKET_V2) {
		slot_id = po->rx_ring.head;
		if (test_bit(slot_id, po->rx_ring.rx_owner_map))
			goto drop_n_account;
		__set_bit(slot_id, po->rx_ring.rx_owner_map);
	}

	if (do_vnet &&
	    virtio_net_hdr_from_skb(skb, h.raw + macoff -
				    sizeof(struct virtio_net_hdr),
				    vio_le(), true, 0)) {
		if (po->tp_version == TPACKET_V3)
			prb_clear_blk_fill_status(&po->rx_ring);
		goto drop_n_account;
	}

	if (po->tp_version <= TPACKET_V2) {
		packet_increment_rx_head(po, &po->rx_ring);
	/*
	 * LOSING will be reported till you read the stats,
	 * because it's COR - Clear On Read.
	 * Anyways, moving it for V1/V2 only as V3 doesn't need this
	 * at packet level.
	 */
		if (atomic_read(&po->tp_drops))
			status |= TP_STATUS_LOSING;
	}

	po->stats.stats1.tp_packets++;
	if (copy_skb) {
		status |= TP_STATUS_COPY;
		__skb_queue_tail(&sk->sk_receive_queue, copy_skb);
	}
	spin_unlock(&sk->sk_receive_queue.lock);

	skb_copy_bits(skb, 0, h.raw + macoff, snaplen);

	if (!(ts_status = tpacket_get_timestamp(skb, &ts, po->tp_tstamp)))
		ktime_get_real_ts64(&ts);

	status |= ts_status;

	switch (po->tp_version) {
	case TPACKET_V1:
		h.h1->tp_len = skb->len;
		h.h1->tp_snaplen = snaplen;
		h.h1->tp_mac = macoff;
		h.h1->tp_net = netoff;
		h.h1->tp_sec = ts.tv_sec;
		h.h1->tp_usec = ts.tv_nsec / NSEC_PER_USEC;
		hdrlen = sizeof(*h.h1);
		break;
	case TPACKET_V2:
		h.h2->tp_len = skb->len;
		h.h2->tp_snaplen = snaplen;
		h.h2->tp_mac = macoff;
		h.h2->tp_net = netoff;
		h.h2->tp_sec = ts.tv_sec;
		h.h2->tp_nsec = ts.tv_nsec;
		if (skb_vlan_tag_present(skb)) {
			h.h2->tp_vlan_tci = skb_vlan_tag_get(skb);
			h.h2->tp_vlan_tpid = ntohs(skb->vlan_proto);
			status |= TP_STATUS_VLAN_VALID | TP_STATUS_VLAN_TPID_VALID;
		} else {
			h.h2->tp_vlan_tci = 0;
			h.h2->tp_vlan_tpid = 0;
		}
		memset(h.h2->tp_padding, 0, sizeof(h.h2->tp_padding));
		hdrlen = sizeof(*h.h2);
		break;
	case TPACKET_V3:
		/* tp_nxt_offset,vlan are already populated above.
		 * So DONT clear those fields here
		 */
		h.h3->tp_status |= status;
		h.h3->tp_len = skb->len;
		h.h3->tp_snaplen = snaplen;
		h.h3->tp_mac = macoff;
		h.h3->tp_net = netoff;
		h.h3->tp_sec  = ts.tv_sec;
		h.h3->tp_nsec = ts.tv_nsec;
		memset(h.h3->tp_padding, 0, sizeof(h.h3->tp_padding));
		hdrlen = sizeof(*h.h3);
		break;
	default:
		BUG();
	}

	sll = h.raw + TPACKET_ALIGN(hdrlen);
	sll->sll_halen = dev_parse_header(skb, sll->sll_addr);
	sll->sll_family = AF_PACKET;
	sll->sll_hatype = dev->type;
	sll->sll_protocol = skb->protocol;
	sll->sll_pkttype = skb->pkt_type;
	if (unlikely(po->origdev))
		sll->sll_ifindex = orig_dev->ifindex;
	else
		sll->sll_ifindex = dev->ifindex;

	smp_mb();

#if ARCH_IMPLEMENTS_FLUSH_DCACHE_PAGE == 1
	if (po->tp_version <= TPACKET_V2) {
		u8 *start, *end;

		end = (u8 *) PAGE_ALIGN((unsigned long) h.raw +
					macoff + snaplen);

		for (start = h.raw; start < end; start += PAGE_SIZE)
			flush_dcache_page(pgv_to_page(start));
	}
	smp_wmb();
#endif

	if (po->tp_version <= TPACKET_V2) {
		spin_lock(&sk->sk_receive_queue.lock);
		__packet_set_status(po, h.raw, status);
		__clear_bit(slot_id, po->rx_ring.rx_owner_map);
		spin_unlock(&sk->sk_receive_queue.lock);
		sk->sk_data_ready(sk);
	} else if (po->tp_version == TPACKET_V3) {
		prb_clear_blk_fill_status(&po->rx_ring);
	}

drop_n_restore:
	if (skb_head != skb->data && skb_shared(skb)) {
		skb->data = skb_head;
		skb->len = skb_len;
	}
drop:
	if (!is_drop_n_account)
		consume_skb(skb);
	else
		kfree_skb(skb);
	return 0;

drop_n_account:
	spin_unlock(&sk->sk_receive_queue.lock);
	atomic_inc(&po->tp_drops);
	is_drop_n_account = true;

	sk->sk_data_ready(sk);
	kfree_skb(copy_skb);
	goto drop_n_restore;
}

static void tpacket_destruct_skb(struct sk_buff *skb)
{
	struct packet_sock *po = pkt_sk(skb->sk);

	if (likely(po->tx_ring.pg_vec)) {
		void *ph;
		__u32 ts;

		ph = skb_zcopy_get_nouarg(skb);
		packet_dec_pending(&po->tx_ring);

		ts = __packet_set_timestamp(po, ph, skb);
		__packet_set_status(po, ph, TP_STATUS_AVAILABLE | ts);

		if (!packet_read_pending(&po->tx_ring))
			complete(&po->skb_completion);
	}

	sock_wfree(skb);
}

static int __packet_snd_vnet_parse(struct virtio_net_hdr *vnet_hdr, size_t len)
{
	if ((vnet_hdr->flags & VIRTIO_NET_HDR_F_NEEDS_CSUM) &&
	    (__virtio16_to_cpu(vio_le(), vnet_hdr->csum_start) +
	     __virtio16_to_cpu(vio_le(), vnet_hdr->csum_offset) + 2 >
	      __virtio16_to_cpu(vio_le(), vnet_hdr->hdr_len)))
		vnet_hdr->hdr_len = __cpu_to_virtio16(vio_le(),
			 __virtio16_to_cpu(vio_le(), vnet_hdr->csum_start) +
			__virtio16_to_cpu(vio_le(), vnet_hdr->csum_offset) + 2);

	if (__virtio16_to_cpu(vio_le(), vnet_hdr->hdr_len) > len)
		return -EINVAL;

	return 0;
}

static int packet_snd_vnet_parse(struct msghdr *msg, size_t *len,
				 struct virtio_net_hdr *vnet_hdr)
{
	if (*len < sizeof(*vnet_hdr))
		return -EINVAL;
	*len -= sizeof(*vnet_hdr);

	if (!copy_from_iter_full(vnet_hdr, sizeof(*vnet_hdr), &msg->msg_iter))
		return -EFAULT;

	return __packet_snd_vnet_parse(vnet_hdr, *len);
}

static int tpacket_fill_skb(struct packet_sock *po, struct sk_buff *skb,
		void *frame, struct net_device *dev, void *data, int tp_len,
		__be16 proto, unsigned char *addr, int hlen, int copylen,
		const struct sockcm_cookie *sockc)
{
	union tpacket_uhdr ph;
	int to_write, offset, len, nr_frags, len_max;
	struct socket *sock = po->sk.sk_socket;
	struct page *page;
	int err;

	ph.raw = frame;

	skb->protocol = proto;
	skb->dev = dev;
	skb->priority = po->sk.sk_priority;
	skb->mark = po->sk.sk_mark;
	skb->tstamp = sockc->transmit_time;
	skb_setup_tx_timestamp(skb, sockc->tsflags);
	skb_zcopy_set_nouarg(skb, ph.raw);

	skb_reserve(skb, hlen);
	skb_reset_network_header(skb);

	to_write = tp_len;

	if (sock->type == SOCK_DGRAM) {
		err = dev_hard_header(skb, dev, ntohs(proto), addr,
				NULL, tp_len);
		if (unlikely(err < 0))
			return -EINVAL;
	} else if (copylen) {
		int hdrlen = min_t(int, copylen, tp_len);

		skb_push(skb, dev->hard_header_len);
		skb_put(skb, copylen - dev->hard_header_len);
		err = skb_store_bits(skb, 0, data, hdrlen);
		if (unlikely(err))
			return err;
		if (!dev_validate_header(dev, skb->data, hdrlen))
			return -EINVAL;

		data += hdrlen;
		to_write -= hdrlen;
	}

	offset = offset_in_page(data);
	len_max = PAGE_SIZE - offset;
	len = ((to_write > len_max) ? len_max : to_write);

	skb->data_len = to_write;
	skb->len += to_write;
	skb->truesize += to_write;
	refcount_add(to_write, &po->sk.sk_wmem_alloc);

	while (likely(to_write)) {
		nr_frags = skb_shinfo(skb)->nr_frags;

		if (unlikely(nr_frags >= MAX_SKB_FRAGS)) {
			pr_err("Packet exceed the number of skb frags(%lu)\n",
			       MAX_SKB_FRAGS);
			return -EFAULT;
		}

		page = pgv_to_page(data);
		data += len;
		flush_dcache_page(page);
		get_page(page);
		skb_fill_page_desc(skb, nr_frags, page, offset, len);
		to_write -= len;
		offset = 0;
		len_max = PAGE_SIZE;
		len = ((to_write > len_max) ? len_max : to_write);
	}

	packet_parse_headers(skb, sock);

	return tp_len;
}

static int tpacket_parse_header(struct packet_sock *po, void *frame,
				int size_max, void **data)
{
	union tpacket_uhdr ph;
	int tp_len, off;

	ph.raw = frame;

	switch (po->tp_version) {
	case TPACKET_V3:
		if (ph.h3->tp_next_offset != 0) {
			pr_warn_once("variable sized slot not supported");
			return -EINVAL;
		}
		tp_len = ph.h3->tp_len;
		break;
	case TPACKET_V2:
		tp_len = ph.h2->tp_len;
		break;
	default:
		tp_len = ph.h1->tp_len;
		break;
	}
	if (unlikely(tp_len > size_max)) {
		pr_err("packet size is too long (%d > %d)\n", tp_len, size_max);
		return -EMSGSIZE;
	}

	if (unlikely(po->tp_tx_has_off)) {
		int off_min, off_max;

		off_min = po->tp_hdrlen - sizeof(struct sockaddr_ll);
		off_max = po->tx_ring.frame_size - tp_len;
		if (po->sk.sk_type == SOCK_DGRAM) {
			switch (po->tp_version) {
			case TPACKET_V3:
				off = ph.h3->tp_net;
				break;
			case TPACKET_V2:
				off = ph.h2->tp_net;
				break;
			default:
				off = ph.h1->tp_net;
				break;
			}
		} else {
			switch (po->tp_version) {
			case TPACKET_V3:
				off = ph.h3->tp_mac;
				break;
			case TPACKET_V2:
				off = ph.h2->tp_mac;
				break;
			default:
				off = ph.h1->tp_mac;
				break;
			}
		}
		if (unlikely((off < off_min) || (off_max < off)))
			return -EINVAL;
	} else {
		off = po->tp_hdrlen - sizeof(struct sockaddr_ll);
	}

	*data = frame + off;
	return tp_len;
}

static int tpacket_snd(struct packet_sock *po, struct msghdr *msg)
{
	struct sk_buff *skb = NULL;
	struct net_device *dev;
	struct virtio_net_hdr *vnet_hdr = NULL;
	struct sockcm_cookie sockc;
	__be16 proto;
	int err, reserve = 0;
	void *ph;
	DECLARE_SOCKADDR(struct sockaddr_ll *, saddr, msg->msg_name);
	bool need_wait = !(msg->msg_flags & MSG_DONTWAIT);
	unsigned char *addr = NULL;
	int tp_len, size_max;
	void *data;
	int len_sum = 0;
	int status = TP_STATUS_AVAILABLE;
	int hlen, tlen, copylen = 0;
	long timeo = 0;

	mutex_lock(&po->pg_vec_lock);

	/* packet_sendmsg() check on tx_ring.pg_vec was lockless,
	 * we need to confirm it under protection of pg_vec_lock.
	 */
	if (unlikely(!po->tx_ring.pg_vec)) {
		err = -EBUSY;
		goto out;
	}
	if (likely(saddr == NULL)) {
		dev	= packet_cached_dev_get(po);
		proto	= po->num;
	} else {
		err = -EINVAL;
		if (msg->msg_namelen < sizeof(struct sockaddr_ll))
			goto out;
		if (msg->msg_namelen < (saddr->sll_halen
					+ offsetof(struct sockaddr_ll,
						sll_addr)))
			goto out;
		proto	= saddr->sll_protocol;
		dev = dev_get_by_index(sock_net(&po->sk), saddr->sll_ifindex);
		if (po->sk.sk_socket->type == SOCK_DGRAM) {
			if (dev && msg->msg_namelen < dev->addr_len +
				   offsetof(struct sockaddr_ll, sll_addr))
				goto out_put;
			addr = saddr->sll_addr;
		}
	}

	err = -ENXIO;
	if (unlikely(dev == NULL))
		goto out;
	err = -ENETDOWN;
	if (unlikely(!(dev->flags & IFF_UP)))
		goto out_put;

	sockcm_init(&sockc, &po->sk);
	if (msg->msg_controllen) {
		err = sock_cmsg_send(&po->sk, msg, &sockc);
		if (unlikely(err))
			goto out_put;
	}

	if (po->sk.sk_socket->type == SOCK_RAW)
		reserve = dev->hard_header_len;
	size_max = po->tx_ring.frame_size
		- (po->tp_hdrlen - sizeof(struct sockaddr_ll));

	if ((size_max > dev->mtu + reserve + VLAN_HLEN) && !po->has_vnet_hdr)
		size_max = dev->mtu + reserve + VLAN_HLEN;

	reinit_completion(&po->skb_completion);

	do {
		ph = packet_current_frame(po, &po->tx_ring,
					  TP_STATUS_SEND_REQUEST);
		if (unlikely(ph == NULL)) {
			if (need_wait && skb) {
				timeo = sock_sndtimeo(&po->sk, msg->msg_flags & MSG_DONTWAIT);
				timeo = wait_for_completion_interruptible_timeout(&po->skb_completion, timeo);
				if (timeo <= 0) {
					err = !timeo ? -ETIMEDOUT : -ERESTARTSYS;
					goto out_put;
				}
			}
			/* check for additional frames */
			continue;
		}

		skb = NULL;
		tp_len = tpacket_parse_header(po, ph, size_max, &data);
		if (tp_len < 0)
			goto tpacket_error;

		status = TP_STATUS_SEND_REQUEST;
		hlen = LL_RESERVED_SPACE(dev);
		tlen = dev->needed_tailroom;
		if (po->has_vnet_hdr) {
			vnet_hdr = data;
			data += sizeof(*vnet_hdr);
			tp_len -= sizeof(*vnet_hdr);
			if (tp_len < 0 ||
			    __packet_snd_vnet_parse(vnet_hdr, tp_len)) {
				tp_len = -EINVAL;
				goto tpacket_error;
			}
			copylen = __virtio16_to_cpu(vio_le(),
						    vnet_hdr->hdr_len);
		}
		copylen = max_t(int, copylen, dev->hard_header_len);
		skb = sock_alloc_send_skb(&po->sk,
				hlen + tlen + sizeof(struct sockaddr_ll) +
				(copylen - dev->hard_header_len),
				!need_wait, &err);

		if (unlikely(skb == NULL)) {
			/* we assume the socket was initially writeable ... */
			if (likely(len_sum > 0))
				err = len_sum;
			goto out_status;
		}
		tp_len = tpacket_fill_skb(po, skb, ph, dev, data, tp_len, proto,
					  addr, hlen, copylen, &sockc);
		if (likely(tp_len >= 0) &&
		    tp_len > dev->mtu + reserve &&
		    !po->has_vnet_hdr &&
		    !packet_extra_vlan_len_allowed(dev, skb))
			tp_len = -EMSGSIZE;

		if (unlikely(tp_len < 0)) {
tpacket_error:
			if (po->tp_loss) {
				__packet_set_status(po, ph,
						TP_STATUS_AVAILABLE);
				packet_increment_head(&po->tx_ring);
				kfree_skb(skb);
				continue;
			} else {
				status = TP_STATUS_WRONG_FORMAT;
				err = tp_len;
				goto out_status;
			}
		}

		if (po->has_vnet_hdr) {
			if (virtio_net_hdr_to_skb(skb, vnet_hdr, vio_le())) {
				tp_len = -EINVAL;
				goto tpacket_error;
			}
			virtio_net_hdr_set_proto(skb, vnet_hdr);
		}

		skb->destructor = tpacket_destruct_skb;
		__packet_set_status(po, ph, TP_STATUS_SENDING);
		packet_inc_pending(&po->tx_ring);

		status = TP_STATUS_SEND_REQUEST;
		err = po->xmit(skb);
		if (unlikely(err > 0)) {
			err = net_xmit_errno(err);
			if (err && __packet_get_status(po, ph) ==
				   TP_STATUS_AVAILABLE) {
				/* skb was destructed already */
				skb = NULL;
				goto out_status;
			}
			/*
			 * skb was dropped but not destructed yet;
			 * let's treat it like congestion or err < 0
			 */
			err = 0;
		}
		packet_increment_head(&po->tx_ring);
		len_sum += tp_len;
	} while (likely((ph != NULL) ||
		/* Note: packet_read_pending() might be slow if we have
		 * to call it as it's per_cpu variable, but in fast-path
		 * we already short-circuit the loop with the first
		 * condition, and luckily don't have to go that path
		 * anyway.
		 */
		 (need_wait && packet_read_pending(&po->tx_ring))));

	err = len_sum;
	goto out_put;

out_status:
	__packet_set_status(po, ph, status);
	kfree_skb(skb);
out_put:
	dev_put(dev);
out:
	mutex_unlock(&po->pg_vec_lock);
	return err;
}

static struct sk_buff *packet_alloc_skb(struct sock *sk, size_t prepad,
				        size_t reserve, size_t len,
				        size_t linear, int noblock,
				        int *err)
{
	struct sk_buff *skb;

	/* Under a page?  Don't bother with paged skb. */
	if (prepad + len < PAGE_SIZE || !linear)
		linear = len;

	skb = sock_alloc_send_pskb(sk, prepad + linear, len - linear, noblock,
				   err, 0);
	if (!skb)
		return NULL;

	skb_reserve(skb, reserve);
	skb_put(skb, linear);
	skb->data_len = len - linear;
	skb->len += len - linear;

	return skb;
}

static int packet_snd(struct socket *sock, struct msghdr *msg, size_t len)
{
	struct sock *sk = sock->sk;
	DECLARE_SOCKADDR(struct sockaddr_ll *, saddr, msg->msg_name);
	struct sk_buff *skb;
	struct net_device *dev;
	__be16 proto;
	unsigned char *addr = NULL;
	int err, reserve = 0;
	struct sockcm_cookie sockc;
	struct virtio_net_hdr vnet_hdr = { 0 };
	int offset = 0;
	struct packet_sock *po = pkt_sk(sk);
	bool has_vnet_hdr = false;
	int hlen, tlen, linear;
	int extra_len = 0;

	/*
	 *	Get and verify the address.
	 */

	if (likely(saddr == NULL)) {
		dev	= packet_cached_dev_get(po);
		proto	= po->num;
	} else {
		err = -EINVAL;
		if (msg->msg_namelen < sizeof(struct sockaddr_ll))
			goto out;
		if (msg->msg_namelen < (saddr->sll_halen + offsetof(struct sockaddr_ll, sll_addr)))
			goto out;
		proto	= saddr->sll_protocol;
		dev = dev_get_by_index(sock_net(sk), saddr->sll_ifindex);
		if (sock->type == SOCK_DGRAM) {
			if (dev && msg->msg_namelen < dev->addr_len +
				   offsetof(struct sockaddr_ll, sll_addr))
				goto out_unlock;
			addr = saddr->sll_addr;
		}
	}

	err = -ENXIO;
	if (unlikely(dev == NULL))
		goto out_unlock;
	err = -ENETDOWN;
	if (unlikely(!(dev->flags & IFF_UP)))
		goto out_unlock;

	sockcm_init(&sockc, sk);
	sockc.mark = sk->sk_mark;
	if (msg->msg_controllen) {
		err = sock_cmsg_send(sk, msg, &sockc);
		if (unlikely(err))
			goto out_unlock;
	}

	if (sock->type == SOCK_RAW)
		reserve = dev->hard_header_len;
	if (po->has_vnet_hdr) {
		err = packet_snd_vnet_parse(msg, &len, &vnet_hdr);
		if (err)
			goto out_unlock;
		has_vnet_hdr = true;
	}

	if (unlikely(sock_flag(sk, SOCK_NOFCS))) {
		if (!netif_supports_nofcs(dev)) {
			err = -EPROTONOSUPPORT;
			goto out_unlock;
		}
		extra_len = 4; /* We're doing our own CRC */
	}

	err = -EMSGSIZE;
	if (!vnet_hdr.gso_type &&
	    (len > dev->mtu + reserve + VLAN_HLEN + extra_len))
		goto out_unlock;

	err = -ENOBUFS;
	hlen = LL_RESERVED_SPACE(dev);
	tlen = dev->needed_tailroom;
	linear = __virtio16_to_cpu(vio_le(), vnet_hdr.hdr_len);
	linear = max(linear, min_t(int, len, dev->hard_header_len));
	skb = packet_alloc_skb(sk, hlen + tlen, hlen, len, linear,
			       msg->msg_flags & MSG_DONTWAIT, &err);
	if (skb == NULL)
		goto out_unlock;

	skb_reset_network_header(skb);

	err = -EINVAL;
	if (sock->type == SOCK_DGRAM) {
		offset = dev_hard_header(skb, dev, ntohs(proto), addr, NULL, len);
		if (unlikely(offset < 0))
			goto out_free;
	} else if (reserve) {
		skb_reserve(skb, -reserve);
		if (len < reserve + sizeof(struct ipv6hdr) &&
		    dev->min_header_len != dev->hard_header_len)
			skb_reset_network_header(skb);
	}

	/* Returns -EFAULT on error */
	err = skb_copy_datagram_from_iter(skb, offset, &msg->msg_iter, len);
	if (err)
		goto out_free;

	if (sock->type == SOCK_RAW &&
	    !dev_validate_header(dev, skb->data, len)) {
		err = -EINVAL;
		goto out_free;
	}

	skb_setup_tx_timestamp(skb, sockc.tsflags);

	if (!vnet_hdr.gso_type && (len > dev->mtu + reserve + extra_len) &&
	    !packet_extra_vlan_len_allowed(dev, skb)) {
		err = -EMSGSIZE;
		goto out_free;
	}

	skb->protocol = proto;
	skb->dev = dev;
	skb->priority = sk->sk_priority;
	skb->mark = sockc.mark;
	skb->tstamp = sockc.transmit_time;

	if (has_vnet_hdr) {
		err = virtio_net_hdr_to_skb(skb, &vnet_hdr, vio_le());
		if (err)
			goto out_free;
		len += sizeof(vnet_hdr);
		virtio_net_hdr_set_proto(skb, &vnet_hdr);
	}

	packet_parse_headers(skb, sock);

	if (unlikely(extra_len == 4))
		skb->no_fcs = 1;

	err = po->xmit(skb);
	if (err > 0 && (err = net_xmit_errno(err)) != 0)
		goto out_unlock;

	dev_put(dev);

	return len;

out_free:
	kfree_skb(skb);
out_unlock:
	if (dev)
		dev_put(dev);
out:
	return err;
}

static int packet_sendmsg(struct socket *sock, struct msghdr *msg, size_t len)
{
	struct sock *sk = sock->sk;
	struct packet_sock *po = pkt_sk(sk);

	if (po->tx_ring.pg_vec)
		return tpacket_snd(po, msg);
	else
		return packet_snd(sock, msg, len);
}

/*
 *	Close a PACKET socket. This is fairly simple. We immediately go
 *	to 'closed' state and remove our protocol entry in the device list.
 */

static int packet_release(struct socket *sock)
{
	struct sock *sk = sock->sk;
	struct packet_sock *po;
	struct packet_fanout *f;
	struct net *net;
	union tpacket_req_u req_u;

	if (!sk)
		return 0;

	net = sock_net(sk);
	po = pkt_sk(sk);

	mutex_lock(&net->packet.sklist_lock);
	sk_del_node_init_rcu(sk);
	mutex_unlock(&net->packet.sklist_lock);

	preempt_disable();
	sock_prot_inuse_add(net, sk->sk_prot, -1);
	preempt_enable();

	spin_lock(&po->bind_lock);
	unregister_prot_hook(sk, false);
	packet_cached_dev_reset(po);

	if (po->prot_hook.dev) {
		dev_put(po->prot_hook.dev);
		po->prot_hook.dev = NULL;
	}
	spin_unlock(&po->bind_lock);

	packet_flush_mclist(sk);

	lock_sock(sk);
	if (po->rx_ring.pg_vec) {
		memset(&req_u, 0, sizeof(req_u));
		packet_set_ring(sk, &req_u, 1, 0);
	}

	if (po->tx_ring.pg_vec) {
		memset(&req_u, 0, sizeof(req_u));
		packet_set_ring(sk, &req_u, 1, 1);
	}
	release_sock(sk);

	f = fanout_release(sk);

	synchronize_net();

	kfree(po->rollover);
	if (f) {
		fanout_release_data(f);
		kfree(f);
	}
	/*
	 *	Now the socket is dead. No more input will appear.
	 */
	sock_orphan(sk);
	sock->sk = NULL;

	/* Purge queues */

	skb_queue_purge(&sk->sk_receive_queue);
	packet_free_pending(po);
	sk_refcnt_debug_release(sk);

	sock_put(sk);
	return 0;
}

/*
 *	Attach a packet hook.
 */

static int packet_do_bind(struct sock *sk, const char *name, int ifindex,
			  __be16 proto)
{
	struct packet_sock *po = pkt_sk(sk);
	struct net_device *dev_curr;
	__be16 proto_curr;
	bool need_rehook;
	struct net_device *dev = NULL;
	int ret = 0;
	bool unlisted = false;

	lock_sock(sk);
	spin_lock(&po->bind_lock);
	rcu_read_lock();

	if (po->fanout) {
		ret = -EINVAL;
		goto out_unlock;
	}

	if (name) {
		dev = dev_get_by_name_rcu(sock_net(sk), name);
		if (!dev) {
			ret = -ENODEV;
			goto out_unlock;
		}
	} else if (ifindex) {
		dev = dev_get_by_index_rcu(sock_net(sk), ifindex);
		if (!dev) {
			ret = -ENODEV;
			goto out_unlock;
		}
	}

	if (dev)
		dev_hold(dev);

	proto_curr = po->prot_hook.type;
	dev_curr = po->prot_hook.dev;

	need_rehook = proto_curr != proto || dev_curr != dev;

	if (need_rehook) {
		if (po->running) {
			rcu_read_unlock();
			/* prevents packet_notifier() from calling
			 * register_prot_hook()
			 */
			po->num = 0;
			__unregister_prot_hook(sk, true);
			rcu_read_lock();
			dev_curr = po->prot_hook.dev;
			if (dev)
				unlisted = !dev_get_by_index_rcu(sock_net(sk),
								 dev->ifindex);
		}

		BUG_ON(po->running);
		po->num = proto;
		po->prot_hook.type = proto;

		if (unlikely(unlisted)) {
			dev_put(dev);
			po->prot_hook.dev = NULL;
			po->ifindex = -1;
			packet_cached_dev_reset(po);
		} else {
			po->prot_hook.dev = dev;
			po->ifindex = dev ? dev->ifindex : 0;
			packet_cached_dev_assign(po, dev);
		}
	}
	if (dev_curr)
		dev_put(dev_curr);

	if (proto == 0 || !need_rehook)
		goto out_unlock;

	if (!unlisted && (!dev || (dev->flags & IFF_UP))) {
		register_prot_hook(sk);
	} else {
		sk->sk_err = ENETDOWN;
		if (!sock_flag(sk, SOCK_DEAD))
			sk->sk_error_report(sk);
	}

out_unlock:
	rcu_read_unlock();
	spin_unlock(&po->bind_lock);
	release_sock(sk);
	return ret;
}

/*
 *	Bind a packet socket to a device
 */

static int packet_bind_spkt(struct socket *sock, struct sockaddr *uaddr,
			    int addr_len)
{
	struct sock *sk = sock->sk;
	char name[sizeof(uaddr->sa_data) + 1];

	/*
	 *	Check legality
	 */

	if (addr_len != sizeof(struct sockaddr))
		return -EINVAL;
	/* uaddr->sa_data comes from the userspace, it's not guaranteed to be
	 * zero-terminated.
	 */
	memcpy(name, uaddr->sa_data, sizeof(uaddr->sa_data));
	name[sizeof(uaddr->sa_data)] = 0;

	return packet_do_bind(sk, name, 0, pkt_sk(sk)->num);
}

static int packet_bind(struct socket *sock, struct sockaddr *uaddr, int addr_len)
{
	struct sockaddr_ll *sll = (struct sockaddr_ll *)uaddr;
	struct sock *sk = sock->sk;

	/*
	 *	Check legality
	 */

	if (addr_len < sizeof(struct sockaddr_ll))
		return -EINVAL;
	if (sll->sll_family != AF_PACKET)
		return -EINVAL;

	return packet_do_bind(sk, NULL, sll->sll_ifindex,
			      sll->sll_protocol ? : pkt_sk(sk)->num);
}

static struct proto packet_proto = {
	.name	  = "PACKET",
	.owner	  = THIS_MODULE,
	.obj_size = sizeof(struct packet_sock),
};

/*
 *	Create a packet of type SOCK_PACKET.
 */

static int packet_create(struct net *net, struct socket *sock, int protocol,
			 int kern)
{
	struct sock *sk;
	struct packet_sock *po;
	__be16 proto = (__force __be16)protocol; /* weird, but documented */
	int err;

	if (!ns_capable(net->user_ns, CAP_NET_RAW))
		return -EPERM;
	if (sock->type != SOCK_DGRAM && sock->type != SOCK_RAW &&
	    sock->type != SOCK_PACKET)
		return -ESOCKTNOSUPPORT;

	sock->state = SS_UNCONNECTED;

	err = -ENOBUFS;
	sk = sk_alloc(net, PF_PACKET, GFP_KERNEL, &packet_proto, kern);
	if (sk == NULL)
		goto out;

	sock->ops = &packet_ops;
	if (sock->type == SOCK_PACKET)
		sock->ops = &packet_ops_spkt;

	sock_init_data(sock, sk);

	po = pkt_sk(sk);
	init_completion(&po->skb_completion);
	sk->sk_family = PF_PACKET;
	po->num = proto;
	po->xmit = dev_queue_xmit;

	err = packet_alloc_pending(po);
	if (err)
		goto out2;

	packet_cached_dev_reset(po);

	sk->sk_destruct = packet_sock_destruct;
	sk_refcnt_debug_inc(sk);

	/*
	 *	Attach a protocol block
	 */

	spin_lock_init(&po->bind_lock);
	mutex_init(&po->pg_vec_lock);
	po->rollover = NULL;
	po->prot_hook.func = packet_rcv;

	if (sock->type == SOCK_PACKET)
		po->prot_hook.func = packet_rcv_spkt;

	po->prot_hook.af_packet_priv = sk;

	if (proto) {
		po->prot_hook.type = proto;
		__register_prot_hook(sk);
	}

	mutex_lock(&net->packet.sklist_lock);
	sk_add_node_tail_rcu(sk, &net->packet.sklist);
	mutex_unlock(&net->packet.sklist_lock);

	preempt_disable();
	sock_prot_inuse_add(net, &packet_proto, 1);
	preempt_enable();

	return 0;
out2:
	sk_free(sk);
out:
	return err;
}

/*
 *	Pull a packet from our receive queue and hand it to the user.
 *	If necessary we block.
 */

static int packet_recvmsg(struct socket *sock, struct msghdr *msg, size_t len,
			  int flags)
{
	struct sock *sk = sock->sk;
	struct sk_buff *skb;
	int copied, err;
	int vnet_hdr_len = 0;
	unsigned int origlen = 0;

	err = -EINVAL;
	if (flags & ~(MSG_PEEK|MSG_DONTWAIT|MSG_TRUNC|MSG_CMSG_COMPAT|MSG_ERRQUEUE))
		goto out;

#if 0
	/* What error should we return now? EUNATTACH? */
	if (pkt_sk(sk)->ifindex < 0)
		return -ENODEV;
#endif

	if (flags & MSG_ERRQUEUE) {
		err = sock_recv_errqueue(sk, msg, len,
					 SOL_PACKET, PACKET_TX_TIMESTAMP);
		goto out;
	}

	/*
	 *	Call the generic datagram receiver. This handles all sorts
	 *	of horrible races and re-entrancy so we can forget about it
	 *	in the protocol layers.
	 *
	 *	Now it will return ENETDOWN, if device have just gone down,
	 *	but then it will block.
	 */

	skb = skb_recv_datagram(sk, flags, flags & MSG_DONTWAIT, &err);

	/*
	 *	An error occurred so return it. Because skb_recv_datagram()
	 *	handles the blocking we don't see and worry about blocking
	 *	retries.
	 */

	if (skb == NULL)
		goto out;

	packet_rcv_try_clear_pressure(pkt_sk(sk));

	if (pkt_sk(sk)->has_vnet_hdr) {
		err = packet_rcv_vnet(msg, skb, &len);
		if (err)
			goto out_free;
		vnet_hdr_len = sizeof(struct virtio_net_hdr);
	}

	/* You lose any data beyond the buffer you gave. If it worries
	 * a user program they can ask the device for its MTU
	 * anyway.
	 */
	copied = skb->len;
	if (copied > len) {
		copied = len;
		msg->msg_flags |= MSG_TRUNC;
	}

	err = skb_copy_datagram_msg(skb, 0, msg, copied);
	if (err)
		goto out_free;

	if (sock->type != SOCK_PACKET) {
		struct sockaddr_ll *sll = &PACKET_SKB_CB(skb)->sa.ll;

		/* Original length was stored in sockaddr_ll fields */
		origlen = PACKET_SKB_CB(skb)->sa.origlen;
		sll->sll_family = AF_PACKET;
		sll->sll_protocol = skb->protocol;
	}

	sock_recv_ts_and_drops(msg, sk, skb);

	if (msg->msg_name) {
		int copy_len;

		/* If the address length field is there to be filled
		 * in, we fill it in now.
		 */
		if (sock->type == SOCK_PACKET) {
			__sockaddr_check_size(sizeof(struct sockaddr_pkt));
			msg->msg_namelen = sizeof(struct sockaddr_pkt);
			copy_len = msg->msg_namelen;
		} else {
			struct sockaddr_ll *sll = &PACKET_SKB_CB(skb)->sa.ll;

			msg->msg_namelen = sll->sll_halen +
				offsetof(struct sockaddr_ll, sll_addr);
			copy_len = msg->msg_namelen;
			if (msg->msg_namelen < sizeof(struct sockaddr_ll)) {
				memset(msg->msg_name +
				       offsetof(struct sockaddr_ll, sll_addr),
				       0, sizeof(sll->sll_addr));
				msg->msg_namelen = sizeof(struct sockaddr_ll);
			}
		}
		memcpy(msg->msg_name, &PACKET_SKB_CB(skb)->sa, copy_len);
	}

	if (pkt_sk(sk)->auxdata) {
		struct tpacket_auxdata aux;

		aux.tp_status = TP_STATUS_USER;
		if (skb->ip_summed == CHECKSUM_PARTIAL)
			aux.tp_status |= TP_STATUS_CSUMNOTREADY;
		else if (skb->pkt_type != PACKET_OUTGOING &&
			 (skb->ip_summed == CHECKSUM_COMPLETE ||
			  skb_csum_unnecessary(skb)))
			aux.tp_status |= TP_STATUS_CSUM_VALID;

		aux.tp_len = origlen;
		aux.tp_snaplen = skb->len;
		aux.tp_mac = 0;
		aux.tp_net = skb_network_offset(skb);
		if (skb_vlan_tag_present(skb)) {
			aux.tp_vlan_tci = skb_vlan_tag_get(skb);
			aux.tp_vlan_tpid = ntohs(skb->vlan_proto);
			aux.tp_status |= TP_STATUS_VLAN_VALID | TP_STATUS_VLAN_TPID_VALID;
		} else {
			aux.tp_vlan_tci = 0;
			aux.tp_vlan_tpid = 0;
		}
		put_cmsg(msg, SOL_PACKET, PACKET_AUXDATA, sizeof(aux), &aux);
	}

	/*
	 *	Free or return the buffer as appropriate. Again this
	 *	hides all the races and re-entrancy issues from us.
	 */
	err = vnet_hdr_len + ((flags&MSG_TRUNC) ? skb->len : copied);

out_free:
	skb_free_datagram(sk, skb);
out:
	return err;
}

static int packet_getname_spkt(struct socket *sock, struct sockaddr *uaddr,
			       int peer)
{
	struct net_device *dev;
	struct sock *sk	= sock->sk;

	if (peer)
		return -EOPNOTSUPP;

	uaddr->sa_family = AF_PACKET;
	memset(uaddr->sa_data, 0, sizeof(uaddr->sa_data));
	rcu_read_lock();
	dev = dev_get_by_index_rcu(sock_net(sk), pkt_sk(sk)->ifindex);
	if (dev)
		strlcpy(uaddr->sa_data, dev->name, sizeof(uaddr->sa_data));
	rcu_read_unlock();

	return sizeof(*uaddr);
}

static int packet_getname(struct socket *sock, struct sockaddr *uaddr,
			  int peer)
{
	struct net_device *dev;
	struct sock *sk = sock->sk;
	struct packet_sock *po = pkt_sk(sk);
	DECLARE_SOCKADDR(struct sockaddr_ll *, sll, uaddr);

	if (peer)
		return -EOPNOTSUPP;

	sll->sll_family = AF_PACKET;
	sll->sll_ifindex = po->ifindex;
	sll->sll_protocol = po->num;
	sll->sll_pkttype = 0;
	rcu_read_lock();
	dev = dev_get_by_index_rcu(sock_net(sk), po->ifindex);
	if (dev) {
		sll->sll_hatype = dev->type;
		sll->sll_halen = dev->addr_len;
		memcpy(sll->sll_addr, dev->dev_addr, dev->addr_len);
	} else {
		sll->sll_hatype = 0;	/* Bad: we have no ARPHRD_UNSPEC */
		sll->sll_halen = 0;
	}
	rcu_read_unlock();

	return offsetof(struct sockaddr_ll, sll_addr) + sll->sll_halen;
}

static int packet_dev_mc(struct net_device *dev, struct packet_mclist *i,
			 int what)
{
	switch (i->type) {
	case PACKET_MR_MULTICAST:
		if (i->alen != dev->addr_len)
			return -EINVAL;
		if (what > 0)
			return dev_mc_add(dev, i->addr);
		else
			return dev_mc_del(dev, i->addr);
		break;
	case PACKET_MR_PROMISC:
		return dev_set_promiscuity(dev, what);
	case PACKET_MR_ALLMULTI:
		return dev_set_allmulti(dev, what);
	case PACKET_MR_UNICAST:
		if (i->alen != dev->addr_len)
			return -EINVAL;
		if (what > 0)
			return dev_uc_add(dev, i->addr);
		else
			return dev_uc_del(dev, i->addr);
		break;
	default:
		break;
	}
	return 0;
}

static void packet_dev_mclist_delete(struct net_device *dev,
				     struct packet_mclist **mlp)
{
	struct packet_mclist *ml;

	while ((ml = *mlp) != NULL) {
		if (ml->ifindex == dev->ifindex) {
			packet_dev_mc(dev, ml, -1);
			*mlp = ml->next;
			kfree(ml);
		} else
			mlp = &ml->next;
	}
}

static int packet_mc_add(struct sock *sk, struct packet_mreq_max *mreq)
{
	struct packet_sock *po = pkt_sk(sk);
	struct packet_mclist *ml, *i;
	struct net_device *dev;
	int err;

	rtnl_lock();

	err = -ENODEV;
	dev = __dev_get_by_index(sock_net(sk), mreq->mr_ifindex);
	if (!dev)
		goto done;

	err = -EINVAL;
	if (mreq->mr_alen > dev->addr_len)
		goto done;

	err = -ENOBUFS;
	i = kmalloc(sizeof(*i), GFP_KERNEL);
	if (i == NULL)
		goto done;

	err = 0;
	for (ml = po->mclist; ml; ml = ml->next) {
		if (ml->ifindex == mreq->mr_ifindex &&
		    ml->type == mreq->mr_type &&
		    ml->alen == mreq->mr_alen &&
		    memcmp(ml->addr, mreq->mr_address, ml->alen) == 0) {
			ml->count++;
			/* Free the new element ... */
			kfree(i);
			goto done;
		}
	}

	i->type = mreq->mr_type;
	i->ifindex = mreq->mr_ifindex;
	i->alen = mreq->mr_alen;
	memcpy(i->addr, mreq->mr_address, i->alen);
	memset(i->addr + i->alen, 0, sizeof(i->addr) - i->alen);
	i->count = 1;
	i->next = po->mclist;
	po->mclist = i;
	err = packet_dev_mc(dev, i, 1);
	if (err) {
		po->mclist = i->next;
		kfree(i);
	}

done:
	rtnl_unlock();
	return err;
}

static int packet_mc_drop(struct sock *sk, struct packet_mreq_max *mreq)
{
	struct packet_mclist *ml, **mlp;

	rtnl_lock();

	for (mlp = &pkt_sk(sk)->mclist; (ml = *mlp) != NULL; mlp = &ml->next) {
		if (ml->ifindex == mreq->mr_ifindex &&
		    ml->type == mreq->mr_type &&
		    ml->alen == mreq->mr_alen &&
		    memcmp(ml->addr, mreq->mr_address, ml->alen) == 0) {
			if (--ml->count == 0) {
				struct net_device *dev;
				*mlp = ml->next;
				dev = __dev_get_by_index(sock_net(sk), ml->ifindex);
				if (dev)
					packet_dev_mc(dev, ml, -1);
				kfree(ml);
			}
			break;
		}
	}
	rtnl_unlock();
	return 0;
}

static void packet_flush_mclist(struct sock *sk)
{
	struct packet_sock *po = pkt_sk(sk);
	struct packet_mclist *ml;

	if (!po->mclist)
		return;

	rtnl_lock();
	while ((ml = po->mclist) != NULL) {
		struct net_device *dev;

		po->mclist = ml->next;
		dev = __dev_get_by_index(sock_net(sk), ml->ifindex);
		if (dev != NULL)
			packet_dev_mc(dev, ml, -1);
		kfree(ml);
	}
	rtnl_unlock();
}

static int
packet_setsockopt(struct socket *sock, int level, int optname, sockptr_t optval,
		  unsigned int optlen)
{
	struct sock *sk = sock->sk;
	struct packet_sock *po = pkt_sk(sk);
	int ret;

	if (level != SOL_PACKET)
		return -ENOPROTOOPT;

	switch (optname) {
	case PACKET_ADD_MEMBERSHIP:
	case PACKET_DROP_MEMBERSHIP:
	{
		struct packet_mreq_max mreq;
		int len = optlen;
		memset(&mreq, 0, sizeof(mreq));
		if (len < sizeof(struct packet_mreq))
			return -EINVAL;
		if (len > sizeof(mreq))
			len = sizeof(mreq);
		if (copy_from_sockptr(&mreq, optval, len))
			return -EFAULT;
		if (len < (mreq.mr_alen + offsetof(struct packet_mreq, mr_address)))
			return -EINVAL;
		if (optname == PACKET_ADD_MEMBERSHIP)
			ret = packet_mc_add(sk, &mreq);
		else
			ret = packet_mc_drop(sk, &mreq);
		return ret;
	}

	case PACKET_RX_RING:
	case PACKET_TX_RING:
	{
		union tpacket_req_u req_u;
		int len;

		lock_sock(sk);
		switch (po->tp_version) {
		case TPACKET_V1:
		case TPACKET_V2:
			len = sizeof(req_u.req);
			break;
		case TPACKET_V3:
		default:
			len = sizeof(req_u.req3);
			break;
		}
		if (optlen < len) {
			ret = -EINVAL;
		} else {
			if (copy_from_sockptr(&req_u.req, optval, len))
				ret = -EFAULT;
			else
				ret = packet_set_ring(sk, &req_u, 0,
						    optname == PACKET_TX_RING);
		}
		release_sock(sk);
		return ret;
	}
	case PACKET_COPY_THRESH:
	{
		int val;

		if (optlen != sizeof(val))
			return -EINVAL;
		if (copy_from_sockptr(&val, optval, sizeof(val)))
			return -EFAULT;

		pkt_sk(sk)->copy_thresh = val;
		return 0;
	}
	case PACKET_VERSION:
	{
		int val;

		if (optlen != sizeof(val))
			return -EINVAL;
		if (copy_from_sockptr(&val, optval, sizeof(val)))
			return -EFAULT;
		switch (val) {
		case TPACKET_V1:
		case TPACKET_V2:
		case TPACKET_V3:
			break;
		default:
			return -EINVAL;
		}
		lock_sock(sk);
		if (po->rx_ring.pg_vec || po->tx_ring.pg_vec) {
			ret = -EBUSY;
		} else {
			po->tp_version = val;
			ret = 0;
		}
		release_sock(sk);
		return ret;
	}
	case PACKET_RESERVE:
	{
		unsigned int val;

		if (optlen != sizeof(val))
			return -EINVAL;
		if (copy_from_sockptr(&val, optval, sizeof(val)))
			return -EFAULT;
		if (val > INT_MAX)
			return -EINVAL;
		lock_sock(sk);
		if (po->rx_ring.pg_vec || po->tx_ring.pg_vec) {
			ret = -EBUSY;
		} else {
			po->tp_reserve = val;
			ret = 0;
		}
		release_sock(sk);
		return ret;
	}
	case PACKET_LOSS:
	{
		unsigned int val;

		if (optlen != sizeof(val))
			return -EINVAL;
		if (copy_from_sockptr(&val, optval, sizeof(val)))
			return -EFAULT;

		lock_sock(sk);
		if (po->rx_ring.pg_vec || po->tx_ring.pg_vec) {
			ret = -EBUSY;
		} else {
			po->tp_loss = !!val;
			ret = 0;
		}
		release_sock(sk);
		return ret;
	}
	case PACKET_AUXDATA:
	{
		int val;

		if (optlen < sizeof(val))
			return -EINVAL;
		if (copy_from_sockptr(&val, optval, sizeof(val)))
			return -EFAULT;

		lock_sock(sk);
		po->auxdata = !!val;
		release_sock(sk);
		return 0;
	}
	case PACKET_ORIGDEV:
	{
		int val;

		if (optlen < sizeof(val))
			return -EINVAL;
		if (copy_from_sockptr(&val, optval, sizeof(val)))
			return -EFAULT;

		lock_sock(sk);
		po->origdev = !!val;
		release_sock(sk);
		return 0;
	}
	case PACKET_VNET_HDR:
	{
		int val;

		if (sock->type != SOCK_RAW)
			return -EINVAL;
		if (optlen < sizeof(val))
			return -EINVAL;
		if (copy_from_sockptr(&val, optval, sizeof(val)))
			return -EFAULT;

		lock_sock(sk);
		if (po->rx_ring.pg_vec || po->tx_ring.pg_vec) {
			ret = -EBUSY;
		} else {
			po->has_vnet_hdr = !!val;
			ret = 0;
		}
		release_sock(sk);
		return ret;
	}
	case PACKET_TIMESTAMP:
	{
		int val;

		if (optlen != sizeof(val))
			return -EINVAL;
		if (copy_from_sockptr(&val, optval, sizeof(val)))
			return -EFAULT;

		po->tp_tstamp = val;
		return 0;
	}
	case PACKET_FANOUT:
	{
		int val;

		if (optlen != sizeof(val))
			return -EINVAL;
		if (copy_from_sockptr(&val, optval, sizeof(val)))
			return -EFAULT;

		return fanout_add(sk, val & 0xffff, val >> 16);
	}
	case PACKET_FANOUT_DATA:
	{
		if (!po->fanout)
			return -EINVAL;

		return fanout_set_data(po, optval, optlen);
	}
	case PACKET_IGNORE_OUTGOING:
	{
		int val;

		if (optlen != sizeof(val))
			return -EINVAL;
		if (copy_from_sockptr(&val, optval, sizeof(val)))
			return -EFAULT;
		if (val < 0 || val > 1)
			return -EINVAL;

		po->prot_hook.ignore_outgoing = !!val;
		return 0;
	}
	case PACKET_TX_HAS_OFF:
	{
		unsigned int val;

		if (optlen != sizeof(val))
			return -EINVAL;
		if (copy_from_sockptr(&val, optval, sizeof(val)))
			return -EFAULT;

		lock_sock(sk);
		if (po->rx_ring.pg_vec || po->tx_ring.pg_vec) {
			ret = -EBUSY;
		} else {
			po->tp_tx_has_off = !!val;
			ret = 0;
		}
		release_sock(sk);
		return 0;
	}
	case PACKET_QDISC_BYPASS:
	{
		int val;

		if (optlen != sizeof(val))
			return -EINVAL;
		if (copy_from_sockptr(&val, optval, sizeof(val)))
			return -EFAULT;

		po->xmit = val ? packet_direct_xmit : dev_queue_xmit;
		return 0;
	}
	default:
		return -ENOPROTOOPT;
	}
}

static int packet_getsockopt(struct socket *sock, int level, int optname,
			     char __user *optval, int __user *optlen)
{
	int len;
	int val, lv = sizeof(val);
	struct sock *sk = sock->sk;
	struct packet_sock *po = pkt_sk(sk);
	void *data = &val;
	union tpacket_stats_u st;
	struct tpacket_rollover_stats rstats;
	int drops;

	if (level != SOL_PACKET)
		return -ENOPROTOOPT;

	if (get_user(len, optlen))
		return -EFAULT;

	if (len < 0)
		return -EINVAL;

	switch (optname) {
	case PACKET_STATISTICS:
		spin_lock_bh(&sk->sk_receive_queue.lock);
		memcpy(&st, &po->stats, sizeof(st));
		memset(&po->stats, 0, sizeof(po->stats));
		spin_unlock_bh(&sk->sk_receive_queue.lock);
		drops = atomic_xchg(&po->tp_drops, 0);

		if (po->tp_version == TPACKET_V3) {
			lv = sizeof(struct tpacket_stats_v3);
			st.stats3.tp_drops = drops;
			st.stats3.tp_packets += drops;
			data = &st.stats3;
		} else {
			lv = sizeof(struct tpacket_stats);
			st.stats1.tp_drops = drops;
			st.stats1.tp_packets += drops;
			data = &st.stats1;
		}

		break;
	case PACKET_AUXDATA:
		val = po->auxdata;
		break;
	case PACKET_ORIGDEV:
		val = po->origdev;
		break;
	case PACKET_VNET_HDR:
		val = po->has_vnet_hdr;
		break;
	case PACKET_VERSION:
		val = po->tp_version;
		break;
	case PACKET_HDRLEN:
		if (len > sizeof(int))
			len = sizeof(int);
		if (len < sizeof(int))
			return -EINVAL;
		if (copy_from_user(&val, optval, len))
			return -EFAULT;
		switch (val) {
		case TPACKET_V1:
			val = sizeof(struct tpacket_hdr);
			break;
		case TPACKET_V2:
			val = sizeof(struct tpacket2_hdr);
			break;
		case TPACKET_V3:
			val = sizeof(struct tpacket3_hdr);
			break;
		default:
			return -EINVAL;
		}
		break;
	case PACKET_RESERVE:
		val = po->tp_reserve;
		break;
	case PACKET_LOSS:
		val = po->tp_loss;
		break;
	case PACKET_TIMESTAMP:
		val = po->tp_tstamp;
		break;
	case PACKET_FANOUT:
		val = (po->fanout ?
		       ((u32)po->fanout->id |
			((u32)po->fanout->type << 16) |
			((u32)po->fanout->flags << 24)) :
		       0);
		break;
	case PACKET_IGNORE_OUTGOING:
		val = po->prot_hook.ignore_outgoing;
		break;
	case PACKET_ROLLOVER_STATS:
		if (!po->rollover)
			return -EINVAL;
		rstats.tp_all = atomic_long_read(&po->rollover->num);
		rstats.tp_huge = atomic_long_read(&po->rollover->num_huge);
		rstats.tp_failed = atomic_long_read(&po->rollover->num_failed);
		data = &rstats;
		lv = sizeof(rstats);
		break;
	case PACKET_TX_HAS_OFF:
		val = po->tp_tx_has_off;
		break;
	case PACKET_QDISC_BYPASS:
		val = packet_use_direct_xmit(po);
		break;
	default:
		return -ENOPROTOOPT;
	}

	if (len > lv)
		len = lv;
	if (put_user(len, optlen))
		return -EFAULT;
	if (copy_to_user(optval, data, len))
		return -EFAULT;
	return 0;
}

static int packet_notifier(struct notifier_block *this,
			   unsigned long msg, void *ptr)
{
	struct sock *sk;
	struct net_device *dev = netdev_notifier_info_to_dev(ptr);
	struct net *net = dev_net(dev);

	rcu_read_lock();
	sk_for_each_rcu(sk, &net->packet.sklist) {
		struct packet_sock *po = pkt_sk(sk);

		switch (msg) {
		case NETDEV_UNREGISTER:
			if (po->mclist)
				packet_dev_mclist_delete(dev, &po->mclist);
			fallthrough;

		case NETDEV_DOWN:
			if (dev->ifindex == po->ifindex) {
				spin_lock(&po->bind_lock);
				if (po->running) {
					__unregister_prot_hook(sk, false);
					sk->sk_err = ENETDOWN;
					if (!sock_flag(sk, SOCK_DEAD))
						sk->sk_error_report(sk);
				}
				if (msg == NETDEV_UNREGISTER) {
					packet_cached_dev_reset(po);
					po->ifindex = -1;
					if (po->prot_hook.dev)
						dev_put(po->prot_hook.dev);
					po->prot_hook.dev = NULL;
				}
				spin_unlock(&po->bind_lock);
			}
			break;
		case NETDEV_UP:
			if (dev->ifindex == po->ifindex) {
				spin_lock(&po->bind_lock);
				if (po->num)
					register_prot_hook(sk);
				spin_unlock(&po->bind_lock);
			}
			break;
		}
	}
	rcu_read_unlock();
	return NOTIFY_DONE;
}


static int packet_ioctl(struct socket *sock, unsigned int cmd,
			unsigned long arg)
{
	struct sock *sk = sock->sk;

	switch (cmd) {
	case SIOCOUTQ:
	{
		int amount = sk_wmem_alloc_get(sk);

		return put_user(amount, (int __user *)arg);
	}
	case SIOCINQ:
	{
		struct sk_buff *skb;
		int amount = 0;

		spin_lock_bh(&sk->sk_receive_queue.lock);
		skb = skb_peek(&sk->sk_receive_queue);
		if (skb)
			amount = skb->len;
		spin_unlock_bh(&sk->sk_receive_queue.lock);
		return put_user(amount, (int __user *)arg);
	}
#ifdef CONFIG_INET
	case SIOCADDRT:
	case SIOCDELRT:
	case SIOCDARP:
	case SIOCGARP:
	case SIOCSARP:
	case SIOCGIFADDR:
	case SIOCSIFADDR:
	case SIOCGIFBRDADDR:
	case SIOCSIFBRDADDR:
	case SIOCGIFNETMASK:
	case SIOCSIFNETMASK:
	case SIOCGIFDSTADDR:
	case SIOCSIFDSTADDR:
	case SIOCSIFFLAGS:
		return inet_dgram_ops.ioctl(sock, cmd, arg);
#endif

	default:
		return -ENOIOCTLCMD;
	}
	return 0;
}

static __poll_t packet_poll(struct file *file, struct socket *sock,
				poll_table *wait)
{
	struct sock *sk = sock->sk;
	struct packet_sock *po = pkt_sk(sk);
	__poll_t mask = datagram_poll(file, sock, wait);

	spin_lock_bh(&sk->sk_receive_queue.lock);
	if (po->rx_ring.pg_vec) {
		if (!packet_previous_rx_frame(po, &po->rx_ring,
			TP_STATUS_KERNEL))
			mask |= EPOLLIN | EPOLLRDNORM;
	}
	packet_rcv_try_clear_pressure(po);
	spin_unlock_bh(&sk->sk_receive_queue.lock);
	spin_lock_bh(&sk->sk_write_queue.lock);
	if (po->tx_ring.pg_vec) {
		if (packet_current_frame(po, &po->tx_ring, TP_STATUS_AVAILABLE))
			mask |= EPOLLOUT | EPOLLWRNORM;
	}
	spin_unlock_bh(&sk->sk_write_queue.lock);
	return mask;
}


/* Dirty? Well, I still did not learn better way to account
 * for user mmaps.
 */

static void packet_mm_open(struct vm_area_struct *vma)
{
	struct file *file = vma->vm_file;
	struct socket *sock = file->private_data;
	struct sock *sk = sock->sk;

	if (sk)
		atomic_inc(&pkt_sk(sk)->mapped);
}

static void packet_mm_close(struct vm_area_struct *vma)
{
	struct file *file = vma->vm_file;
	struct socket *sock = file->private_data;
	struct sock *sk = sock->sk;

	if (sk)
		atomic_dec(&pkt_sk(sk)->mapped);
}

static const struct vm_operations_struct packet_mmap_ops = {
	.open	=	packet_mm_open,
	.close	=	packet_mm_close,
};

static void free_pg_vec(struct pgv *pg_vec, unsigned int order,
			unsigned int len)
{
	int i;

	for (i = 0; i < len; i++) {
		if (likely(pg_vec[i].buffer)) {
			if (is_vmalloc_addr(pg_vec[i].buffer))
				vfree(pg_vec[i].buffer);
			else
				free_pages((unsigned long)pg_vec[i].buffer,
					   order);
			pg_vec[i].buffer = NULL;
		}
	}
	kfree(pg_vec);
}

static char *alloc_one_pg_vec_page(unsigned long order)
{
	char *buffer;
	gfp_t gfp_flags = GFP_KERNEL | __GFP_COMP |
			  __GFP_ZERO | __GFP_NOWARN | __GFP_NORETRY;

	buffer = (char *) __get_free_pages(gfp_flags, order);
	if (buffer)
		return buffer;

	/* __get_free_pages failed, fall back to vmalloc */
	buffer = vzalloc(array_size((1 << order), PAGE_SIZE));
	if (buffer)
		return buffer;

	/* vmalloc failed, lets dig into swap here */
	gfp_flags &= ~__GFP_NORETRY;
	buffer = (char *) __get_free_pages(gfp_flags, order);
	if (buffer)
		return buffer;

	/* complete and utter failure */
	return NULL;
}

static struct pgv *alloc_pg_vec(struct tpacket_req *req, int order)
{
	unsigned int block_nr = req->tp_block_nr;
	struct pgv *pg_vec;
	int i;

	pg_vec = kcalloc(block_nr, sizeof(struct pgv), GFP_KERNEL | __GFP_NOWARN);
	if (unlikely(!pg_vec))
		goto out;

	for (i = 0; i < block_nr; i++) {
		pg_vec[i].buffer = alloc_one_pg_vec_page(order);
		if (unlikely(!pg_vec[i].buffer))
			goto out_free_pgvec;
	}

out:
	return pg_vec;

out_free_pgvec:
	free_pg_vec(pg_vec, order, block_nr);
	pg_vec = NULL;
	goto out;
}

static int packet_set_ring(struct sock *sk, union tpacket_req_u *req_u,
		int closing, int tx_ring)
{
	struct pgv *pg_vec = NULL;
	struct packet_sock *po = pkt_sk(sk);
	unsigned long *rx_owner_map = NULL;
	int was_running, order = 0;
	struct packet_ring_buffer *rb;
	struct sk_buff_head *rb_queue;
	__be16 num;
	int err;
	/* Added to avoid minimal code churn */
	struct tpacket_req *req = &req_u->req;

	rb = tx_ring ? &po->tx_ring : &po->rx_ring;
	rb_queue = tx_ring ? &sk->sk_write_queue : &sk->sk_receive_queue;

	err = -EBUSY;
	if (!closing) {
		if (atomic_read(&po->mapped))
			goto out;
		if (packet_read_pending(rb))
			goto out;
	}

	if (req->tp_block_nr) {
		unsigned int min_frame_size;

		/* Sanity tests and some calculations */
		err = -EBUSY;
		if (unlikely(rb->pg_vec))
			goto out;

		switch (po->tp_version) {
		case TPACKET_V1:
			po->tp_hdrlen = TPACKET_HDRLEN;
			break;
		case TPACKET_V2:
			po->tp_hdrlen = TPACKET2_HDRLEN;
			break;
		case TPACKET_V3:
			po->tp_hdrlen = TPACKET3_HDRLEN;
			break;
		}

		err = -EINVAL;
		if (unlikely((int)req->tp_block_size <= 0))
			goto out;
		if (unlikely(!PAGE_ALIGNED(req->tp_block_size)))
			goto out;
		min_frame_size = po->tp_hdrlen + po->tp_reserve;
		if (po->tp_version >= TPACKET_V3 &&
		    req->tp_block_size <
		    BLK_PLUS_PRIV((u64)req_u->req3.tp_sizeof_priv) + min_frame_size)
			goto out;
		if (unlikely(req->tp_frame_size < min_frame_size))
			goto out;
		if (unlikely(req->tp_frame_size & (TPACKET_ALIGNMENT - 1)))
			goto out;

		rb->frames_per_block = req->tp_block_size / req->tp_frame_size;
		if (unlikely(rb->frames_per_block == 0))
			goto out;
		if (unlikely(rb->frames_per_block > UINT_MAX / req->tp_block_nr))
			goto out;
		if (unlikely((rb->frames_per_block * req->tp_block_nr) !=
					req->tp_frame_nr))
			goto out;

		err = -ENOMEM;
		order = get_order(req->tp_block_size);
		pg_vec = alloc_pg_vec(req, order);
		if (unlikely(!pg_vec))
			goto out;
		switch (po->tp_version) {
		case TPACKET_V3:
			/* Block transmit is not supported yet */
			if (!tx_ring) {
				init_prb_bdqc(po, rb, pg_vec, req_u);
			} else {
				struct tpacket_req3 *req3 = &req_u->req3;

				if (req3->tp_retire_blk_tov ||
				    req3->tp_sizeof_priv ||
				    req3->tp_feature_req_word) {
					err = -EINVAL;
					goto out_free_pg_vec;
				}
			}
			break;
		default:
			if (!tx_ring) {
				rx_owner_map = bitmap_alloc(req->tp_frame_nr,
					GFP_KERNEL | __GFP_NOWARN | __GFP_ZERO);
				if (!rx_owner_map)
					goto out_free_pg_vec;
			}
			break;
		}
	}
	/* Done */
	else {
		err = -EINVAL;
		if (unlikely(req->tp_frame_nr))
			goto out;
	}


	/* Detach socket from network */
	spin_lock(&po->bind_lock);
	was_running = po->running;
	num = po->num;
	if (was_running) {
		po->num = 0;
		__unregister_prot_hook(sk, false);
	}
	spin_unlock(&po->bind_lock);

	synchronize_net();

	err = -EBUSY;
	mutex_lock(&po->pg_vec_lock);
	if (closing || atomic_read(&po->mapped) == 0) {
		err = 0;
		spin_lock_bh(&rb_queue->lock);
		swap(rb->pg_vec, pg_vec);
		if (po->tp_version <= TPACKET_V2)
			swap(rb->rx_owner_map, rx_owner_map);
		rb->frame_max = (req->tp_frame_nr - 1);
		rb->head = 0;
		rb->frame_size = req->tp_frame_size;
		spin_unlock_bh(&rb_queue->lock);

		swap(rb->pg_vec_order, order);
		swap(rb->pg_vec_len, req->tp_block_nr);

		rb->pg_vec_pages = req->tp_block_size/PAGE_SIZE;
		po->prot_hook.func = (po->rx_ring.pg_vec) ?
						tpacket_rcv : packet_rcv;
		skb_queue_purge(rb_queue);
		if (atomic_read(&po->mapped))
			pr_err("packet_mmap: vma is busy: %d\n",
			       atomic_read(&po->mapped));
	}
	mutex_unlock(&po->pg_vec_lock);

	spin_lock(&po->bind_lock);
	if (was_running) {
		po->num = num;
		register_prot_hook(sk);
	}
	spin_unlock(&po->bind_lock);
	if (pg_vec && (po->tp_version > TPACKET_V2)) {
		/* Because we don't support block-based V3 on tx-ring */
		if (!tx_ring)
			prb_shutdown_retire_blk_timer(po, rb_queue);
	}

out_free_pg_vec:
	bitmap_free(rx_owner_map);
	if (pg_vec)
		free_pg_vec(pg_vec, order, req->tp_block_nr);
out:
	return err;
}

static int packet_mmap(struct file *file, struct socket *sock,
		struct vm_area_struct *vma)
{
	struct sock *sk = sock->sk;
	struct packet_sock *po = pkt_sk(sk);
	unsigned long size, expected_size;
	struct packet_ring_buffer *rb;
	unsigned long start;
	int err = -EINVAL;
	int i;

	if (vma->vm_pgoff)
		return -EINVAL;

	mutex_lock(&po->pg_vec_lock);

	expected_size = 0;
	for (rb = &po->rx_ring; rb <= &po->tx_ring; rb++) {
		if (rb->pg_vec) {
			expected_size += rb->pg_vec_len
						* rb->pg_vec_pages
						* PAGE_SIZE;
		}
	}

	if (expected_size == 0)
		goto out;

	size = vma->vm_end - vma->vm_start;
	if (size != expected_size)
		goto out;

	start = vma->vm_start;
	for (rb = &po->rx_ring; rb <= &po->tx_ring; rb++) {
		if (rb->pg_vec == NULL)
			continue;

		for (i = 0; i < rb->pg_vec_len; i++) {
			struct page *page;
			void *kaddr = rb->pg_vec[i].buffer;
			int pg_num;

			for (pg_num = 0; pg_num < rb->pg_vec_pages; pg_num++) {
				page = pgv_to_page(kaddr);
				err = vm_insert_page(vma, start, page);
				if (unlikely(err))
					goto out;
				start += PAGE_SIZE;
				kaddr += PAGE_SIZE;
			}
		}
	}

	atomic_inc(&po->mapped);
	vma->vm_ops = &packet_mmap_ops;
	err = 0;

out:
	mutex_unlock(&po->pg_vec_lock);
	return err;
}

static const struct proto_ops packet_ops_spkt = {
	.family =	PF_PACKET,
	.owner =	THIS_MODULE,
	.release =	packet_release,
	.bind =		packet_bind_spkt,
	.connect =	sock_no_connect,
	.socketpair =	sock_no_socketpair,
	.accept =	sock_no_accept,
	.getname =	packet_getname_spkt,
	.poll =		datagram_poll,
	.ioctl =	packet_ioctl,
	.gettstamp =	sock_gettstamp,
	.listen =	sock_no_listen,
	.shutdown =	sock_no_shutdown,
	.sendmsg =	packet_sendmsg_spkt,
	.recvmsg =	packet_recvmsg,
	.mmap =		sock_no_mmap,
	.sendpage =	sock_no_sendpage,
};

static const struct proto_ops packet_ops = {
	.family =	PF_PACKET,
	.owner =	THIS_MODULE,
	.release =	packet_release,
	.bind =		packet_bind,
	.connect =	sock_no_connect,
	.socketpair =	sock_no_socketpair,
	.accept =	sock_no_accept,
	.getname =	packet_getname,
	.poll =		packet_poll,
	.ioctl =	packet_ioctl,
	.gettstamp =	sock_gettstamp,
	.listen =	sock_no_listen,
	.shutdown =	sock_no_shutdown,
	.setsockopt =	packet_setsockopt,
	.getsockopt =	packet_getsockopt,
	.sendmsg =	packet_sendmsg,
	.recvmsg =	packet_recvmsg,
	.mmap =		packet_mmap,
	.sendpage =	sock_no_sendpage,
};

static const struct net_proto_family packet_family_ops = {
	.family =	PF_PACKET,
	.create =	packet_create,
	.owner	=	THIS_MODULE,
};

static struct notifier_block packet_netdev_notifier = {
	.notifier_call =	packet_notifier,
};

#ifdef CONFIG_PROC_FS

static void *packet_seq_start(struct seq_file *seq, loff_t *pos)
	__acquires(RCU)
{
	struct net *net = seq_file_net(seq);

	rcu_read_lock();
	return seq_hlist_start_head_rcu(&net->packet.sklist, *pos);
}

static void *packet_seq_next(struct seq_file *seq, void *v, loff_t *pos)
{
	struct net *net = seq_file_net(seq);
	return seq_hlist_next_rcu(v, &net->packet.sklist, pos);
}

static void packet_seq_stop(struct seq_file *seq, void *v)
	__releases(RCU)
{
	rcu_read_unlock();
}

static int packet_seq_show(struct seq_file *seq, void *v)
{
	if (v == SEQ_START_TOKEN)
		seq_puts(seq, "sk       RefCnt Type Proto  Iface R Rmem   User   Inode\n");
	else {
		struct sock *s = sk_entry(v);
		const struct packet_sock *po = pkt_sk(s);

		seq_printf(seq,
			   "%pK %-6d %-4d %04x   %-5d %1d %-6u %-6u %-6lu\n",
			   s,
			   refcount_read(&s->sk_refcnt),
			   s->sk_type,
			   ntohs(po->num),
			   po->ifindex,
			   po->running,
			   atomic_read(&s->sk_rmem_alloc),
			   from_kuid_munged(seq_user_ns(seq), sock_i_uid(s)),
			   sock_i_ino(s));
	}

	return 0;
}

static const struct seq_operations packet_seq_ops = {
	.start	= packet_seq_start,
	.next	= packet_seq_next,
	.stop	= packet_seq_stop,
	.show	= packet_seq_show,
};
#endif

static int __net_init packet_net_init(struct net *net)
{
	mutex_init(&net->packet.sklist_lock);
	INIT_HLIST_HEAD(&net->packet.sklist);

	if (!proc_create_net("packet", 0, net->proc_net, &packet_seq_ops,
			sizeof(struct seq_net_private)))
		return -ENOMEM;

	return 0;
}

static void __net_exit packet_net_exit(struct net *net)
{
	remove_proc_entry("packet", net->proc_net);
	WARN_ON_ONCE(!hlist_empty(&net->packet.sklist));
}

static struct pernet_operations packet_net_ops = {
	.init = packet_net_init,
	.exit = packet_net_exit,
};


static void __exit packet_exit(void)
{
	unregister_netdevice_notifier(&packet_netdev_notifier);
	unregister_pernet_subsys(&packet_net_ops);
	sock_unregister(PF_PACKET);
	proto_unregister(&packet_proto);
}

static int __init packet_init(void)
{
	int rc;

	rc = proto_register(&packet_proto, 0);
	if (rc)
		goto out;
	rc = sock_register(&packet_family_ops);
	if (rc)
		goto out_proto;
	rc = register_pernet_subsys(&packet_net_ops);
	if (rc)
		goto out_sock;
	rc = register_netdevice_notifier(&packet_netdev_notifier);
	if (rc)
		goto out_pernet;

	return 0;

out_pernet:
	unregister_pernet_subsys(&packet_net_ops);
out_sock:
	sock_unregister(PF_PACKET);
out_proto:
	proto_unregister(&packet_proto);
out:
	return rc;
}

module_init(packet_init);
module_exit(packet_exit);
MODULE_LICENSE("GPL");
MODULE_ALIAS_NETPROTO(PF_PACKET);