	atomic_dec(&netbk->netfront_count);
}

static void xen_netbk_idx_release(struct xen_netbk *netbk, u16 pending_idx);
static void make_tx_response(struct xenvif *vif,
			     struct xen_netif_tx_request *txp,
			     s8       st);
static struct xen_netif_rx_response *make_rx_response(struct xenvif *vif,

	do {
		make_tx_response(vif, txp, XEN_NETIF_RSP_ERROR);
		if (cons >= end)
			break;
		txp = RING_GET_REQUEST(&vif->tx, cons++);
	} while (1);
	vif->tx.req_cons = cons;
	xenvif_put(vif);
}

static int netbk_count_requests(struct xenvif *vif,
				struct xen_netif_tx_request *first,
				struct xen_netif_tx_request *txp,
				int work_to_do)

	do {
		if (frags >= work_to_do) {
			netdev_dbg(vif->dev, "Need more frags\n");
			return -frags;
		}

		if (unlikely(frags >= MAX_SKB_FRAGS)) {
			netdev_dbg(vif->dev, "Too many frags\n");
			return -frags;
		}

		memcpy(txp, RING_GET_REQUEST(&vif->tx, cons + frags),
		       sizeof(*txp));
		if (txp->size > first->size) {
			netdev_dbg(vif->dev, "Frags galore\n");
			return -frags;
		}

		first->size -= txp->size;
		frags++;

		if (unlikely((txp->offset + txp->size) > PAGE_SIZE)) {
			netdev_dbg(vif->dev, "txp->offset: %x, size: %u\n",
				 txp->offset, txp->size);
			return -frags;
		}
	} while ((txp++)->flags & XEN_NETTXF_more_data);
	return frags;
		pending_idx = netbk->pending_ring[index];
		page = xen_netbk_alloc_page(netbk, skb, pending_idx);
		if (!page)
			return NULL;

		gop->source.u.ref = txp->gref;
		gop->source.domid = vif->domid;
		gop->source.offset = txp->offset;
	}

	return gop;
}

static int xen_netbk_tx_check_gop(struct xen_netbk *netbk,
				  struct sk_buff *skb,
{
	struct gnttab_copy *gop = *gopp;
	u16 pending_idx = *((u16 *)skb->data);
	struct pending_tx_info *pending_tx_info = netbk->pending_tx_info;
	struct xenvif *vif = pending_tx_info[pending_idx].vif;
	struct xen_netif_tx_request *txp;
	struct skb_shared_info *shinfo = skb_shinfo(skb);
	int nr_frags = shinfo->nr_frags;
	int i, err, start;

	/* Check status of header. */
	err = gop->status;
	if (unlikely(err)) {
		pending_ring_idx_t index;
		index = pending_index(netbk->pending_prod++);
		txp = &pending_tx_info[pending_idx].req;
		make_tx_response(vif, txp, XEN_NETIF_RSP_ERROR);
		netbk->pending_ring[index] = pending_idx;
		xenvif_put(vif);
	}

	/* Skip first skb fragment if it is on same page as header fragment. */
	start = (frag_get_pending_idx(&shinfo->frags[0]) == pending_idx);

	for (i = start; i < nr_frags; i++) {
		int j, newerr;
		pending_ring_idx_t index;

		pending_idx = frag_get_pending_idx(&shinfo->frags[i]);

		/* Check error status: if okay then remember grant handle. */
		if (likely(!newerr)) {
			/* Had a previous error? Invalidate this fragment. */
			if (unlikely(err))
				xen_netbk_idx_release(netbk, pending_idx);
			continue;
		}

		/* Error on this fragment: respond to client with an error. */
		txp = &netbk->pending_tx_info[pending_idx].req;
		make_tx_response(vif, txp, XEN_NETIF_RSP_ERROR);
		index = pending_index(netbk->pending_prod++);
		netbk->pending_ring[index] = pending_idx;
		xenvif_put(vif);

		/* Not the first error? Preceding frags already invalidated. */
		if (err)
			continue;

		/* First error: invalidate header and preceding fragments. */
		pending_idx = *((u16 *)skb->data);
		xen_netbk_idx_release(netbk, pending_idx);
		for (j = start; j < i; j++) {
			pending_idx = frag_get_pending_idx(&shinfo->frags[j]);
			xen_netbk_idx_release(netbk, pending_idx);
		}

		/* Remember the error: invalidate all subsequent fragments. */
		err = newerr;

		/* Take an extra reference to offset xen_netbk_idx_release */
		get_page(netbk->mmap_pages[pending_idx]);
		xen_netbk_idx_release(netbk, pending_idx);
	}
}

static int xen_netbk_get_extras(struct xenvif *vif,

	do {
		if (unlikely(work_to_do-- <= 0)) {
			netdev_dbg(vif->dev, "Missing extra info\n");
			return -EBADR;
		}

		memcpy(&extra, RING_GET_REQUEST(&vif->tx, cons),
		if (unlikely(!extra.type ||
			     extra.type >= XEN_NETIF_EXTRA_TYPE_MAX)) {
			vif->tx.req_cons = ++cons;
			netdev_dbg(vif->dev,
				   "Invalid extra type: %d\n", extra.type);
			return -EINVAL;
		}

		memcpy(&extras[extra.type - 1], &extra, sizeof(extra));
			     struct xen_netif_extra_info *gso)
{
	if (!gso->u.gso.size) {
		netdev_dbg(vif->dev, "GSO size must not be zero.\n");
		return -EINVAL;
	}

	/* Currently only TCPv4 S.O. is supported. */
	if (gso->u.gso.type != XEN_NETIF_GSO_TYPE_TCPV4) {
		netdev_dbg(vif->dev, "Bad GSO type %d.\n", gso->u.gso.type);
		return -EINVAL;
	}

	skb_shinfo(skb)->gso_size = gso->u.gso.size;

		/* Get a netif from the list with work to do. */
		vif = poll_net_schedule_list(netbk);
		if (!vif)
			continue;

		RING_FINAL_CHECK_FOR_REQUESTS(&vif->tx, work_to_do);
		if (!work_to_do) {
			xenvif_put(vif);
			continue;
			work_to_do = xen_netbk_get_extras(vif, extras,
							  work_to_do);
			idx = vif->tx.req_cons;
			if (unlikely(work_to_do < 0)) {
				netbk_tx_err(vif, &txreq, idx);
				continue;
			}
		}

		ret = netbk_count_requests(vif, &txreq, txfrags, work_to_do);
		if (unlikely(ret < 0)) {
			netbk_tx_err(vif, &txreq, idx - ret);
			continue;
		}
		idx += ret;

		if (unlikely(txreq.size < ETH_HLEN)) {
			netdev_dbg(vif->dev,

		/* No crossing a page as the payload mustn't fragment. */
		if (unlikely((txreq.offset + txreq.size) > PAGE_SIZE)) {
			netdev_dbg(vif->dev,
				   "txreq.offset: %x, size: %u, end: %lu\n",
				   txreq.offset, txreq.size,
				   (txreq.offset&~PAGE_MASK) + txreq.size);
			netbk_tx_err(vif, &txreq, idx);
			continue;
		}

		index = pending_index(netbk->pending_cons);
			gso = &extras[XEN_NETIF_EXTRA_TYPE_GSO - 1];

			if (netbk_set_skb_gso(vif, skb, gso)) {
				kfree_skb(skb);
				netbk_tx_err(vif, &txreq, idx);
				continue;
			}
		}

			txp->size -= data_len;
		} else {
			/* Schedule a response immediately. */
			xen_netbk_idx_release(netbk, pending_idx);
		}

		if (txp->flags & XEN_NETTXF_csum_blank)
			skb->ip_summed = CHECKSUM_PARTIAL;
	xen_netbk_tx_submit(netbk);
}

static void xen_netbk_idx_release(struct xen_netbk *netbk, u16 pending_idx)
{
	struct xenvif *vif;
	struct pending_tx_info *pending_tx_info;
	pending_ring_idx_t index;

	vif = pending_tx_info->vif;

	make_tx_response(vif, &pending_tx_info->req, XEN_NETIF_RSP_OKAY);

	index = pending_index(netbk->pending_prod++);
	netbk->pending_ring[index] = pending_idx;
