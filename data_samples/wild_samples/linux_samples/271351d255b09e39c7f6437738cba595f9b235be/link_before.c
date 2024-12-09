		if (unlikely(msg_in_group(hdr) || msg_mcast(hdr))) {
			skb_queue_tail(mc_inputq, skb);
			return true;
		}
		fallthrough;
	case CONN_MANAGER:
		skb_queue_tail(inputq, skb);
		return true;
	case GROUP_PROTOCOL:
		skb_queue_tail(mc_inputq, skb);
		return true;
	case NAME_DISTRIBUTOR:
		l->bc_rcvlink->state = LINK_ESTABLISHED;
		skb_queue_tail(l->namedq, skb);
		return true;
	case MSG_BUNDLER:
	case TUNNEL_PROTOCOL:
	case MSG_FRAGMENTER:
	case BCAST_PROTOCOL:
		return false;
#ifdef CONFIG_TIPC_CRYPTO
	case MSG_CRYPTO:
		tipc_crypto_msg_rcv(l->net, skb);
		return true;
#endif
	default:
		pr_warn("Dropping received illegal msg type\n");
		kfree_skb(skb);
		return true;
	}
}

/* tipc_link_input - process packet that has passed link protocol check
 *
 * Consumes buffer
 */
static int tipc_link_input(struct tipc_link *l, struct sk_buff *skb,
			   struct sk_buff_head *inputq,
			   struct sk_buff **reasm_skb)
{
	struct tipc_msg *hdr = buf_msg(skb);
	struct sk_buff *iskb;
	struct sk_buff_head tmpq;
	int usr = msg_user(hdr);
	int pos = 0;

	if (usr == MSG_BUNDLER) {
		skb_queue_head_init(&tmpq);
		l->stats.recv_bundles++;
		l->stats.recv_bundled += msg_msgcnt(hdr);
		while (tipc_msg_extract(skb, &iskb, &pos))
			tipc_data_input(l, iskb, &tmpq);
		tipc_skb_queue_splice_tail(&tmpq, inputq);
		return 0;
	} else if (usr == MSG_FRAGMENTER) {
		l->stats.recv_fragments++;
		if (tipc_buf_append(reasm_skb, &skb)) {
			l->stats.recv_fragmented++;
			tipc_data_input(l, skb, inputq);
		} else if (!*reasm_skb && !link_is_bc_rcvlink(l)) {
			pr_warn_ratelimited("Unable to build fragment list\n");
			return tipc_link_fsm_evt(l, LINK_FAILURE_EVT);
		}
		return 0;
	} else if (usr == BCAST_PROTOCOL) {
		tipc_bcast_lock(l->net);
		tipc_link_bc_init_rcv(l->bc_rcvlink, hdr);
		tipc_bcast_unlock(l->net);
	}

	kfree_skb(skb);
	return 0;
}

/* tipc_link_tnl_rcv() - receive TUNNEL_PROTOCOL message, drop or process the
 *			 inner message along with the ones in the old link's
 *			 deferdq
 * @l: tunnel link
 * @skb: TUNNEL_PROTOCOL message
 * @inputq: queue to put messages ready for delivery
 */
static int tipc_link_tnl_rcv(struct tipc_link *l, struct sk_buff *skb,
			     struct sk_buff_head *inputq)
{
	struct sk_buff **reasm_skb = &l->failover_reasm_skb;
	struct sk_buff **reasm_tnlmsg = &l->reasm_tnlmsg;
	struct sk_buff_head *fdefq = &l->failover_deferdq;
	struct tipc_msg *hdr = buf_msg(skb);
	struct sk_buff *iskb;
	int ipos = 0;
	int rc = 0;
	u16 seqno;

	if (msg_type(hdr) == SYNCH_MSG) {
		kfree_skb(skb);
		return 0;
	}

	/* Not a fragment? */
	if (likely(!msg_nof_fragms(hdr))) {
		if (unlikely(!tipc_msg_extract(skb, &iskb, &ipos))) {
			pr_warn_ratelimited("Unable to extract msg, defq: %d\n",
					    skb_queue_len(fdefq));
			return 0;
		}
		kfree_skb(skb);
	} else {
		/* Set fragment type for buf_append */
		if (msg_fragm_no(hdr) == 1)
			msg_set_type(hdr, FIRST_FRAGMENT);
		else if (msg_fragm_no(hdr) < msg_nof_fragms(hdr))
			msg_set_type(hdr, FRAGMENT);
		else
			msg_set_type(hdr, LAST_FRAGMENT);

		if (!tipc_buf_append(reasm_tnlmsg, &skb)) {
			/* Successful but non-complete reassembly? */
			if (*reasm_tnlmsg || link_is_bc_rcvlink(l))
				return 0;
			pr_warn_ratelimited("Unable to reassemble tunnel msg\n");
			return tipc_link_fsm_evt(l, LINK_FAILURE_EVT);
		}
		iskb = skb;
	}

	do {
		seqno = buf_seqno(iskb);
		if (unlikely(less(seqno, l->drop_point))) {
			kfree_skb(iskb);
			continue;
		}
		if (unlikely(seqno != l->drop_point)) {
			__tipc_skb_queue_sorted(fdefq, seqno, iskb);
			continue;
		}

		l->drop_point++;
		if (!tipc_data_input(l, iskb, inputq))
			rc |= tipc_link_input(l, iskb, inputq, reasm_skb);
		if (unlikely(rc))
			break;
	} while ((iskb = __tipc_skb_dequeue(fdefq, l->drop_point)));

	return rc;
}

/**
 * tipc_get_gap_ack_blks - get Gap ACK blocks from PROTOCOL/STATE_MSG
 * @ga: returned pointer to the Gap ACK blocks if any
 * @l: the tipc link
 * @hdr: the PROTOCOL/STATE_MSG header
 * @uc: desired Gap ACK blocks type, i.e. unicast (= 1) or broadcast (= 0)
 *
 * Return: the total Gap ACK blocks size
 */
u16 tipc_get_gap_ack_blks(struct tipc_gap_ack_blks **ga, struct tipc_link *l,
			  struct tipc_msg *hdr, bool uc)
{
	struct tipc_gap_ack_blks *p;
	u16 sz = 0;

	/* Does peer support the Gap ACK blocks feature? */
	if (l->peer_caps & TIPC_GAP_ACK_BLOCK) {
		p = (struct tipc_gap_ack_blks *)msg_data(hdr);
		sz = ntohs(p->len);
		/* Sanity check */
		if (sz == struct_size(p, gacks, p->ugack_cnt + p->bgack_cnt)) {
			/* Good, check if the desired type exists */
			if ((uc && p->ugack_cnt) || (!uc && p->bgack_cnt))
				goto ok;
		/* Backward compatible: peer might not support bc, but uc? */
		} else if (uc && sz == struct_size(p, gacks, p->ugack_cnt)) {
			if (p->ugack_cnt) {
				p->bgack_cnt = 0;
				goto ok;
			}
		}
	}
	/* Other cases: ignore! */
	p = NULL;

ok:
	*ga = p;
	return sz;
}

static u8 __tipc_build_gap_ack_blks(struct tipc_gap_ack_blks *ga,
				    struct tipc_link *l, u8 start_index)
{
	struct tipc_gap_ack *gacks = &ga->gacks[start_index];
	struct sk_buff *skb = skb_peek(&l->deferdq);
	u16 expect, seqno = 0;
	u8 n = 0;

	if (!skb)
		return 0;

	expect = buf_seqno(skb);
	skb_queue_walk(&l->deferdq, skb) {
		seqno = buf_seqno(skb);
		if (unlikely(more(seqno, expect))) {
			gacks[n].ack = htons(expect - 1);
			gacks[n].gap = htons(seqno - expect);
			if (++n >= MAX_GAP_ACK_BLKS / 2) {
				pr_info_ratelimited("Gacks on %s: %d, ql: %d!\n",
						    l->name, n,
						    skb_queue_len(&l->deferdq));
				return n;
			}
		} else if (unlikely(less(seqno, expect))) {
			pr_warn("Unexpected skb in deferdq!\n");
			continue;
		}
		expect = seqno + 1;
	}

	/* last block */
	gacks[n].ack = htons(seqno);
	gacks[n].gap = 0;
	n++;
	return n;
}

/* tipc_build_gap_ack_blks - build Gap ACK blocks
 * @l: tipc unicast link
 * @hdr: the tipc message buffer to store the Gap ACK blocks after built
 *
 * The function builds Gap ACK blocks for both the unicast & broadcast receiver
 * links of a certain peer, the buffer after built has the network data format
 * as found at the struct tipc_gap_ack_blks definition.
 *
 * returns the actual allocated memory size
 */
static u16 tipc_build_gap_ack_blks(struct tipc_link *l, struct tipc_msg *hdr)
{
	struct tipc_link *bcl = l->bc_rcvlink;
	struct tipc_gap_ack_blks *ga;
	u16 len;

	ga = (struct tipc_gap_ack_blks *)msg_data(hdr);

	/* Start with broadcast link first */
	tipc_bcast_lock(bcl->net);
	msg_set_bcast_ack(hdr, bcl->rcv_nxt - 1);
	msg_set_bc_gap(hdr, link_bc_rcv_gap(bcl));
	ga->bgack_cnt = __tipc_build_gap_ack_blks(ga, bcl, 0);
	tipc_bcast_unlock(bcl->net);

	/* Now for unicast link, but an explicit NACK only (???) */
	ga->ugack_cnt = (msg_seq_gap(hdr)) ?
			__tipc_build_gap_ack_blks(ga, l, ga->bgack_cnt) : 0;

	/* Total len */
	len = struct_size(ga, gacks, ga->bgack_cnt + ga->ugack_cnt);
	ga->len = htons(len);
	return len;
}

/* tipc_link_advance_transmq - advance TIPC link transmq queue by releasing
 *			       acked packets, also doing retransmissions if
 *			       gaps found
 * @l: tipc link with transmq queue to be advanced
 * @r: tipc link "receiver" i.e. in case of broadcast (= "l" if unicast)
 * @acked: seqno of last packet acked by peer without any gaps before
 * @gap: # of gap packets
 * @ga: buffer pointer to Gap ACK blocks from peer
 * @xmitq: queue for accumulating the retransmitted packets if any
 * @retransmitted: returned boolean value if a retransmission is really issued
 * @rc: returned code e.g. TIPC_LINK_DOWN_EVT if a repeated retransmit failures
 *      happens (- unlikely case)
 *
 * Return: the number of packets released from the link transmq
 */
static int tipc_link_advance_transmq(struct tipc_link *l, struct tipc_link *r,
				     u16 acked, u16 gap,
				     struct tipc_gap_ack_blks *ga,
				     struct sk_buff_head *xmitq,
				     bool *retransmitted, int *rc)
{
	struct tipc_gap_ack_blks *last_ga = r->last_ga, *this_ga = NULL;
	struct tipc_gap_ack *gacks = NULL;
	struct sk_buff *skb, *_skb, *tmp;
	struct tipc_msg *hdr;
	u32 qlen = skb_queue_len(&l->transmq);
	u16 nacked = acked, ngap = gap, gack_cnt = 0;
	u16 bc_ack = l->bc_rcvlink->rcv_nxt - 1;
	u16 ack = l->rcv_nxt - 1;
	u16 seqno, n = 0;
	u16 end = r->acked, start = end, offset = r->last_gap;
	u16 si = (last_ga) ? last_ga->start_index : 0;
	bool is_uc = !link_is_bc_sndlink(l);
	bool bc_has_acked = false;

	trace_tipc_link_retrans(r, acked + 1, acked + gap, &l->transmq);

	/* Determine Gap ACK blocks if any for the particular link */
	if (ga && is_uc) {
		/* Get the Gap ACKs, uc part */
		gack_cnt = ga->ugack_cnt;
		gacks = &ga->gacks[ga->bgack_cnt];
	} else if (ga) {
		/* Copy the Gap ACKs, bc part, for later renewal if needed */
		this_ga = kmemdup(ga, struct_size(ga, gacks, ga->bgack_cnt),
				  GFP_ATOMIC);
		if (likely(this_ga)) {
			this_ga->start_index = 0;
			/* Start with the bc Gap ACKs */
			gack_cnt = this_ga->bgack_cnt;
			gacks = &this_ga->gacks[0];
		} else {
			/* Hmm, we can get in trouble..., simply ignore it */
			pr_warn_ratelimited("Ignoring bc Gap ACKs, no memory\n");
		}
	}

	/* Advance the link transmq */
	skb_queue_walk_safe(&l->transmq, skb, tmp) {
		seqno = buf_seqno(skb);

next_gap_ack:
		if (less_eq(seqno, nacked)) {
			if (is_uc)
				goto release;
			/* Skip packets peer has already acked */
			if (!more(seqno, r->acked))
				continue;
			/* Get the next of last Gap ACK blocks */
			while (more(seqno, end)) {
				if (!last_ga || si >= last_ga->bgack_cnt)
					break;
				start = end + offset + 1;
				end = ntohs(last_ga->gacks[si].ack);
				offset = ntohs(last_ga->gacks[si].gap);
				si++;
				WARN_ONCE(more(start, end) ||
					  (!offset &&
					   si < last_ga->bgack_cnt) ||
					  si > MAX_GAP_ACK_BLKS,
					  "Corrupted Gap ACK: %d %d %d %d %d\n",
					  start, end, offset, si,
					  last_ga->bgack_cnt);
			}
			/* Check against the last Gap ACK block */
			if (in_range(seqno, start, end))
				continue;
			/* Update/release the packet peer is acking */
			bc_has_acked = true;
			if (--TIPC_SKB_CB(skb)->ackers)
				continue;
release:
			/* release skb */
			__skb_unlink(skb, &l->transmq);
			kfree_skb(skb);
		} else if (less_eq(seqno, nacked + ngap)) {
			/* First gap: check if repeated retrans failures? */
			if (unlikely(seqno == acked + 1 &&
				     link_retransmit_failure(l, r, rc))) {
				/* Ignore this bc Gap ACKs if any */
				kfree(this_ga);
				this_ga = NULL;
				break;
			}
			/* retransmit skb if unrestricted*/
			if (time_before(jiffies, TIPC_SKB_CB(skb)->nxt_retr))
				continue;
			tipc_link_set_skb_retransmit_time(skb, l);
			_skb = pskb_copy(skb, GFP_ATOMIC);
			if (!_skb)
				continue;
			hdr = buf_msg(_skb);
			msg_set_ack(hdr, ack);
			msg_set_bcast_ack(hdr, bc_ack);
			_skb->priority = TC_PRIO_CONTROL;
			__skb_queue_tail(xmitq, _skb);
			l->stats.retransmitted++;
			if (!is_uc)
				r->stats.retransmitted++;
			*retransmitted = true;
			/* Increase actual retrans counter & mark first time */
			if (!TIPC_SKB_CB(skb)->retr_cnt++)
				TIPC_SKB_CB(skb)->retr_stamp = jiffies;
		} else {
			/* retry with Gap ACK blocks if any */
			if (n >= gack_cnt)
				break;
			nacked = ntohs(gacks[n].ack);
			ngap = ntohs(gacks[n].gap);
			n++;
			goto next_gap_ack;
		}
	}

	/* Renew last Gap ACK blocks for bc if needed */
	if (bc_has_acked) {
		if (this_ga) {
			kfree(last_ga);
			r->last_ga = this_ga;
			r->last_gap = gap;
		} else if (last_ga) {
			if (less(acked, start)) {
				si--;
				offset = start - acked - 1;
			} else if (less(acked, end)) {
				acked = end;
			}
			if (si < last_ga->bgack_cnt) {
				last_ga->start_index = si;
				r->last_gap = offset;
			} else {
				kfree(last_ga);
				r->last_ga = NULL;
				r->last_gap = 0;
			}
		} else {
			r->last_gap = 0;
		}
		r->acked = acked;
	} else {
		kfree(this_ga);
	}

	return qlen - skb_queue_len(&l->transmq);
}

/* tipc_link_build_state_msg: prepare link state message for transmission
 *
 * Note that sending of broadcast ack is coordinated among nodes, to reduce
 * risk of ack storms towards the sender
 */
int tipc_link_build_state_msg(struct tipc_link *l, struct sk_buff_head *xmitq)
{
	if (!l)
		return 0;

	/* Broadcast ACK must be sent via a unicast link => defer to caller */
	if (link_is_bc_rcvlink(l)) {
		if (((l->rcv_nxt ^ tipc_own_addr(l->net)) & 0xf) != 0xf)
			return 0;
		l->rcv_unacked = 0;

		/* Use snd_nxt to store peer's snd_nxt in broadcast rcv link */
		l->snd_nxt = l->rcv_nxt;
		return TIPC_LINK_SND_STATE;
	}
	/* Unicast ACK */
	l->rcv_unacked = 0;
	l->stats.sent_acks++;
	tipc_link_build_proto_msg(l, STATE_MSG, 0, 0, 0, 0, 0, xmitq);
	return 0;
}

/* tipc_link_build_reset_msg: prepare link RESET or ACTIVATE message
 */
void tipc_link_build_reset_msg(struct tipc_link *l, struct sk_buff_head *xmitq)
{
	int mtyp = RESET_MSG;
	struct sk_buff *skb;

	if (l->state == LINK_ESTABLISHING)
		mtyp = ACTIVATE_MSG;

	tipc_link_build_proto_msg(l, mtyp, 0, 0, 0, 0, 0, xmitq);

	/* Inform peer that this endpoint is going down if applicable */
	skb = skb_peek_tail(xmitq);
	if (skb && (l->state == LINK_RESET))
		msg_set_peer_stopping(buf_msg(skb), 1);
}

/* tipc_link_build_nack_msg: prepare link nack message for transmission
 * Note that sending of broadcast NACK is coordinated among nodes, to
 * reduce the risk of NACK storms towards the sender
 */
static int tipc_link_build_nack_msg(struct tipc_link *l,
				    struct sk_buff_head *xmitq)
{
	u32 def_cnt = ++l->stats.deferred_recv;
	struct sk_buff_head *dfq = &l->deferdq;
	u32 defq_len = skb_queue_len(dfq);
	int match1, match2;

	if (link_is_bc_rcvlink(l)) {
		match1 = def_cnt & 0xf;
		match2 = tipc_own_addr(l->net) & 0xf;
		if (match1 == match2)
			return TIPC_LINK_SND_STATE;
		return 0;
	}

	if (defq_len >= 3 && !((defq_len - 3) % 16)) {
		u16 rcvgap = buf_seqno(skb_peek(dfq)) - l->rcv_nxt;

		tipc_link_build_proto_msg(l, STATE_MSG, 0, 0,
					  rcvgap, 0, 0, xmitq);
	}
	return 0;
}

/* tipc_link_rcv - process TIPC packets/messages arriving from off-node
 * @l: the link that should handle the message
 * @skb: TIPC packet
 * @xmitq: queue to place packets to be sent after this call
 */
int tipc_link_rcv(struct tipc_link *l, struct sk_buff *skb,
		  struct sk_buff_head *xmitq)
{
	struct sk_buff_head *defq = &l->deferdq;
	struct tipc_msg *hdr = buf_msg(skb);
	u16 seqno, rcv_nxt, win_lim;
	int released = 0;
	int rc = 0;

	/* Verify and update link state */
	if (unlikely(msg_user(hdr) == LINK_PROTOCOL))
		return tipc_link_proto_rcv(l, skb, xmitq);

	/* Don't send probe at next timeout expiration */
	l->silent_intv_cnt = 0;

	do {
		hdr = buf_msg(skb);
		seqno = msg_seqno(hdr);
		rcv_nxt = l->rcv_nxt;
		win_lim = rcv_nxt + TIPC_MAX_LINK_WIN;

		if (unlikely(!link_is_up(l))) {
			if (l->state == LINK_ESTABLISHING)
				rc = TIPC_LINK_UP_EVT;
			kfree_skb(skb);
			break;
		}

		/* Drop if outside receive window */
		if (unlikely(less(seqno, rcv_nxt) || more(seqno, win_lim))) {
			l->stats.duplicates++;
			kfree_skb(skb);
			break;
		}
		released += tipc_link_advance_transmq(l, l, msg_ack(hdr), 0,
						      NULL, NULL, NULL, NULL);

		/* Defer delivery if sequence gap */
		if (unlikely(seqno != rcv_nxt)) {
			if (!__tipc_skb_queue_sorted(defq, seqno, skb))
				l->stats.duplicates++;
			rc |= tipc_link_build_nack_msg(l, xmitq);
			break;
		}

		/* Deliver packet */
		l->rcv_nxt++;
		l->stats.recv_pkts++;

		if (unlikely(msg_user(hdr) == TUNNEL_PROTOCOL))
			rc |= tipc_link_tnl_rcv(l, skb, l->inputq);
		else if (!tipc_data_input(l, skb, l->inputq))
			rc |= tipc_link_input(l, skb, l->inputq, &l->reasm_buf);
		if (unlikely(++l->rcv_unacked >= TIPC_MIN_LINK_WIN))
			rc |= tipc_link_build_state_msg(l, xmitq);
		if (unlikely(rc & ~TIPC_LINK_SND_STATE))
			break;
	} while ((skb = __tipc_skb_dequeue(defq, l->rcv_nxt)));

	/* Forward queues and wake up waiting users */
	if (released) {
		tipc_link_update_cwin(l, released, 0);
		tipc_link_advance_backlog(l, xmitq);
		if (unlikely(!skb_queue_empty(&l->wakeupq)))
			link_prepare_wakeup(l);
	}
	return rc;
}

static void tipc_link_build_proto_msg(struct tipc_link *l, int mtyp, bool probe,
				      bool probe_reply, u16 rcvgap,
				      int tolerance, int priority,
				      struct sk_buff_head *xmitq)
{
	struct tipc_mon_state *mstate = &l->mon_state;
	struct sk_buff_head *dfq = &l->deferdq;
	struct tipc_link *bcl = l->bc_rcvlink;
	struct tipc_msg *hdr;
	struct sk_buff *skb;
	bool node_up = link_is_up(bcl);
	u16 glen = 0, bc_rcvgap = 0;
	int dlen = 0;
	void *data;

	/* Don't send protocol message during reset or link failover */
	if (tipc_link_is_blocked(l))
		return;

	if (!tipc_link_is_up(l) && (mtyp == STATE_MSG))
		return;

	if ((probe || probe_reply) && !skb_queue_empty(dfq))
		rcvgap = buf_seqno(skb_peek(dfq)) - l->rcv_nxt;

	skb = tipc_msg_create(LINK_PROTOCOL, mtyp, INT_H_SIZE,
			      tipc_max_domain_size + MAX_GAP_ACK_BLKS_SZ,
			      l->addr, tipc_own_addr(l->net), 0, 0, 0);
	if (!skb)
		return;

	hdr = buf_msg(skb);
	data = msg_data(hdr);
	msg_set_session(hdr, l->session);
	msg_set_bearer_id(hdr, l->bearer_id);
	msg_set_net_plane(hdr, l->net_plane);
	msg_set_next_sent(hdr, l->snd_nxt);
	msg_set_ack(hdr, l->rcv_nxt - 1);
	msg_set_bcast_ack(hdr, bcl->rcv_nxt - 1);
	msg_set_bc_ack_invalid(hdr, !node_up);
	msg_set_last_bcast(hdr, l->bc_sndlink->snd_nxt - 1);
	msg_set_link_tolerance(hdr, tolerance);
	msg_set_linkprio(hdr, priority);
	msg_set_redundant_link(hdr, node_up);
	msg_set_seq_gap(hdr, 0);
	msg_set_seqno(hdr, l->snd_nxt + U16_MAX / 2);

	if (mtyp == STATE_MSG) {
		if (l->peer_caps & TIPC_LINK_PROTO_SEQNO)
			msg_set_seqno(hdr, l->snd_nxt_state++);
		msg_set_seq_gap(hdr, rcvgap);
		bc_rcvgap = link_bc_rcv_gap(bcl);
		msg_set_bc_gap(hdr, bc_rcvgap);
		msg_set_probe(hdr, probe);
		msg_set_is_keepalive(hdr, probe || probe_reply);
		if (l->peer_caps & TIPC_GAP_ACK_BLOCK)
			glen = tipc_build_gap_ack_blks(l, hdr);
		tipc_mon_prep(l->net, data + glen, &dlen, mstate, l->bearer_id);
		msg_set_size(hdr, INT_H_SIZE + glen + dlen);
		skb_trim(skb, INT_H_SIZE + glen + dlen);
		l->stats.sent_states++;
		l->rcv_unacked = 0;
	} else {
		/* RESET_MSG or ACTIVATE_MSG */
		if (mtyp == ACTIVATE_MSG) {
			msg_set_dest_session_valid(hdr, 1);
			msg_set_dest_session(hdr, l->peer_session);
		}
		msg_set_max_pkt(hdr, l->advertised_mtu);
		strcpy(data, l->if_name);
		msg_set_size(hdr, INT_H_SIZE + TIPC_MAX_IF_NAME);
		skb_trim(skb, INT_H_SIZE + TIPC_MAX_IF_NAME);
	}
	if (probe)
		l->stats.sent_probes++;
	if (rcvgap)
		l->stats.sent_nacks++;
	if (bc_rcvgap)
		bcl->stats.sent_nacks++;
	skb->priority = TC_PRIO_CONTROL;
	__skb_queue_tail(xmitq, skb);
	trace_tipc_proto_build(skb, false, l->name);
}

void tipc_link_create_dummy_tnl_msg(struct tipc_link *l,
				    struct sk_buff_head *xmitq)
{
	u32 onode = tipc_own_addr(l->net);
	struct tipc_msg *hdr, *ihdr;
	struct sk_buff_head tnlq;
	struct sk_buff *skb;
	u32 dnode = l->addr;

	__skb_queue_head_init(&tnlq);
	skb = tipc_msg_create(TUNNEL_PROTOCOL, FAILOVER_MSG,
			      INT_H_SIZE, BASIC_H_SIZE,
			      dnode, onode, 0, 0, 0);
	if (!skb) {
		pr_warn("%sunable to create tunnel packet\n", link_co_err);
		return;
	}

	hdr = buf_msg(skb);
	msg_set_msgcnt(hdr, 1);
	msg_set_bearer_id(hdr, l->peer_bearer_id);

	ihdr = (struct tipc_msg *)msg_data(hdr);
	tipc_msg_init(onode, ihdr, TIPC_LOW_IMPORTANCE, TIPC_DIRECT_MSG,
		      BASIC_H_SIZE, dnode);
	msg_set_errcode(ihdr, TIPC_ERR_NO_PORT);
	__skb_queue_tail(&tnlq, skb);
	tipc_link_xmit(l, &tnlq, xmitq);
}

/* tipc_link_tnl_prepare(): prepare and return a list of tunnel packets
 * with contents of the link's transmit and backlog queues.
 */
void tipc_link_tnl_prepare(struct tipc_link *l, struct tipc_link *tnl,
			   int mtyp, struct sk_buff_head *xmitq)
{
	struct sk_buff_head *fdefq = &tnl->failover_deferdq;
	struct sk_buff *skb, *tnlskb;
	struct tipc_msg *hdr, tnlhdr;
	struct sk_buff_head *queue = &l->transmq;
	struct sk_buff_head tmpxq, tnlq, frags;
	u16 pktlen, pktcnt, seqno = l->snd_nxt;
	bool pktcnt_need_update = false;
	u16 syncpt;
	int rc;

	if (!tnl)
		return;

	__skb_queue_head_init(&tnlq);
	/* Link Synching:
	 * From now on, send only one single ("dummy") SYNCH message
	 * to peer. The SYNCH message does not contain any data, just
	 * a header conveying the synch point to the peer.
	 */
	if (mtyp == SYNCH_MSG && (tnl->peer_caps & TIPC_TUNNEL_ENHANCED)) {
		tnlskb = tipc_msg_create(TUNNEL_PROTOCOL, SYNCH_MSG,
					 INT_H_SIZE, 0, l->addr,
					 tipc_own_addr(l->net),
					 0, 0, 0);
		if (!tnlskb) {
			pr_warn("%sunable to create dummy SYNCH_MSG\n",
				link_co_err);
			return;
		}

		hdr = buf_msg(tnlskb);
		syncpt = l->snd_nxt + skb_queue_len(&l->backlogq) - 1;
		msg_set_syncpt(hdr, syncpt);
		msg_set_bearer_id(hdr, l->peer_bearer_id);
		__skb_queue_tail(&tnlq, tnlskb);
		tipc_link_xmit(tnl, &tnlq, xmitq);
		return;
	}

	__skb_queue_head_init(&tmpxq);
	__skb_queue_head_init(&frags);
	/* At least one packet required for safe algorithm => add dummy */
	skb = tipc_msg_create(TIPC_LOW_IMPORTANCE, TIPC_DIRECT_MSG,
			      BASIC_H_SIZE, 0, l->addr, tipc_own_addr(l->net),
			      0, 0, TIPC_ERR_NO_PORT);
	if (!skb) {
		pr_warn("%sunable to create tunnel packet\n", link_co_err);
		return;
	}
	__skb_queue_tail(&tnlq, skb);
	tipc_link_xmit(l, &tnlq, &tmpxq);
	__skb_queue_purge(&tmpxq);

	/* Initialize reusable tunnel packet header */
	tipc_msg_init(tipc_own_addr(l->net), &tnlhdr, TUNNEL_PROTOCOL,
		      mtyp, INT_H_SIZE, l->addr);
	if (mtyp == SYNCH_MSG)
		pktcnt = l->snd_nxt - buf_seqno(skb_peek(&l->transmq));
	else
		pktcnt = skb_queue_len(&l->transmq);
	pktcnt += skb_queue_len(&l->backlogq);
	msg_set_msgcnt(&tnlhdr, pktcnt);
	msg_set_bearer_id(&tnlhdr, l->peer_bearer_id);
tnl:
	/* Wrap each packet into a tunnel packet */
	skb_queue_walk(queue, skb) {
		hdr = buf_msg(skb);
		if (queue == &l->backlogq)
			msg_set_seqno(hdr, seqno++);
		pktlen = msg_size(hdr);

		/* Tunnel link MTU is not large enough? This could be
		 * due to:
		 * 1) Link MTU has just changed or set differently;
		 * 2) Or FAILOVER on the top of a SYNCH message
		 *
		 * The 2nd case should not happen if peer supports
		 * TIPC_TUNNEL_ENHANCED
		 */
		if (pktlen > tnl->mtu - INT_H_SIZE) {
			if (mtyp == FAILOVER_MSG &&
			    (tnl->peer_caps & TIPC_TUNNEL_ENHANCED)) {
				rc = tipc_msg_fragment(skb, &tnlhdr, tnl->mtu,
						       &frags);
				if (rc) {
					pr_warn("%sunable to frag msg: rc %d\n",
						link_co_err, rc);
					return;
				}
				pktcnt += skb_queue_len(&frags) - 1;
				pktcnt_need_update = true;
				skb_queue_splice_tail_init(&frags, &tnlq);
				continue;
			}
			/* Unluckily, peer doesn't have TIPC_TUNNEL_ENHANCED
			 * => Just warn it and return!
			 */
			pr_warn_ratelimited("%stoo large msg <%d, %d>: %d!\n",
					    link_co_err, msg_user(hdr),
					    msg_type(hdr), msg_size(hdr));
			return;
		}

		msg_set_size(&tnlhdr, pktlen + INT_H_SIZE);
		tnlskb = tipc_buf_acquire(pktlen + INT_H_SIZE, GFP_ATOMIC);
		if (!tnlskb) {
			pr_warn("%sunable to send packet\n", link_co_err);
			return;
		}
		skb_copy_to_linear_data(tnlskb, &tnlhdr, INT_H_SIZE);
		skb_copy_to_linear_data_offset(tnlskb, INT_H_SIZE, hdr, pktlen);
		__skb_queue_tail(&tnlq, tnlskb);
	}
	if (queue != &l->backlogq) {
		queue = &l->backlogq;
		goto tnl;
	}

	if (pktcnt_need_update)
		skb_queue_walk(&tnlq, skb) {
			hdr = buf_msg(skb);
			msg_set_msgcnt(hdr, pktcnt);
		}

	tipc_link_xmit(tnl, &tnlq, xmitq);

	if (mtyp == FAILOVER_MSG) {
		tnl->drop_point = l->rcv_nxt;
		tnl->failover_reasm_skb = l->reasm_buf;
		l->reasm_buf = NULL;

		/* Failover the link's deferdq */
		if (unlikely(!skb_queue_empty(fdefq))) {
			pr_warn("Link failover deferdq not empty: %d!\n",
				skb_queue_len(fdefq));
			__skb_queue_purge(fdefq);
		}
		skb_queue_splice_init(&l->deferdq, fdefq);
	}
}

/**
 * tipc_link_failover_prepare() - prepare tnl for link failover
 *
 * This is a special version of the precursor - tipc_link_tnl_prepare(),
 * see the tipc_node_link_failover() for details
 *
 * @l: failover link
 * @tnl: tunnel link
 * @xmitq: queue for messages to be xmited
 */
void tipc_link_failover_prepare(struct tipc_link *l, struct tipc_link *tnl,
				struct sk_buff_head *xmitq)
{
	struct sk_buff_head *fdefq = &tnl->failover_deferdq;

	tipc_link_create_dummy_tnl_msg(tnl, xmitq);

	/* This failover link endpoint was never established before,
	 * so it has not received anything from peer.
	 * Otherwise, it must be a normal failover situation or the
	 * node has entered SELF_DOWN_PEER_LEAVING and both peer nodes
	 * would have to start over from scratch instead.
	 */
	tnl->drop_point = 1;
	tnl->failover_reasm_skb = NULL;

	/* Initiate the link's failover deferdq */
	if (unlikely(!skb_queue_empty(fdefq))) {
		pr_warn("Link failover deferdq not empty: %d!\n",
			skb_queue_len(fdefq));
		__skb_queue_purge(fdefq);
	}
}

/* tipc_link_validate_msg(): validate message against current link state
 * Returns true if message should be accepted, otherwise false
 */
bool tipc_link_validate_msg(struct tipc_link *l, struct tipc_msg *hdr)
{
	u16 curr_session = l->peer_session;
	u16 session = msg_session(hdr);
	int mtyp = msg_type(hdr);

	if (msg_user(hdr) != LINK_PROTOCOL)
		return true;

	switch (mtyp) {
	case RESET_MSG:
		if (!l->in_session)
			return true;
		/* Accept only RESET with new session number */
		return more(session, curr_session);
	case ACTIVATE_MSG:
		if (!l->in_session)
			return true;
		/* Accept only ACTIVATE with new or current session number */
		return !less(session, curr_session);
	case STATE_MSG:
		/* Accept only STATE with current session number */
		if (!l->in_session)
			return false;
		if (session != curr_session)
			return false;
		/* Extra sanity check */
		if (!link_is_up(l) && msg_ack(hdr))
			return false;
		if (!(l->peer_caps & TIPC_LINK_PROTO_SEQNO))
			return true;
		/* Accept only STATE with new sequence number */
		return !less(msg_seqno(hdr), l->rcv_nxt_state);
	default:
		return false;
	}
}

/* tipc_link_proto_rcv(): receive link level protocol message :
 * Note that network plane id propagates through the network, and may
 * change at any time. The node with lowest numerical id determines
 * network plane
 */
static int tipc_link_proto_rcv(struct tipc_link *l, struct sk_buff *skb,
			       struct sk_buff_head *xmitq)
{
	struct tipc_msg *hdr = buf_msg(skb);
	struct tipc_gap_ack_blks *ga = NULL;
	bool reply = msg_probe(hdr), retransmitted = false;
	u16 dlen = msg_data_sz(hdr), glen = 0;
	u16 peers_snd_nxt =  msg_next_sent(hdr);
	u16 peers_tol = msg_link_tolerance(hdr);
	u16 peers_prio = msg_linkprio(hdr);
	u16 gap = msg_seq_gap(hdr);
	u16 ack = msg_ack(hdr);
	u16 rcv_nxt = l->rcv_nxt;
	u16 rcvgap = 0;
	int mtyp = msg_type(hdr);
	int rc = 0, released;
	char *if_name;
	void *data;

	trace_tipc_proto_rcv(skb, false, l->name);
	if (tipc_link_is_blocked(l) || !xmitq)
		goto exit;

	if (tipc_own_addr(l->net) > msg_prevnode(hdr))
		l->net_plane = msg_net_plane(hdr);

	skb_linearize(skb);
	hdr = buf_msg(skb);
	data = msg_data(hdr);

	if (!tipc_link_validate_msg(l, hdr)) {
		trace_tipc_skb_dump(skb, false, "PROTO invalid (1)!");
		trace_tipc_link_dump(l, TIPC_DUMP_NONE, "PROTO invalid (1)!");
		goto exit;
	}

	switch (mtyp) {
	case RESET_MSG:
	case ACTIVATE_MSG:
		/* Complete own link name with peer's interface name */
		if_name =  strrchr(l->name, ':') + 1;
		if (sizeof(l->name) - (if_name - l->name) <= TIPC_MAX_IF_NAME)
			break;
		if (msg_data_sz(hdr) < TIPC_MAX_IF_NAME)
			break;
		strncpy(if_name, data, TIPC_MAX_IF_NAME);

		/* Update own tolerance if peer indicates a non-zero value */
		if (in_range(peers_tol, TIPC_MIN_LINK_TOL, TIPC_MAX_LINK_TOL)) {
			l->tolerance = peers_tol;
			l->bc_rcvlink->tolerance = peers_tol;
		}
		/* Update own priority if peer's priority is higher */
		if (in_range(peers_prio, l->priority + 1, TIPC_MAX_LINK_PRI))
			l->priority = peers_prio;

		/* If peer is going down we want full re-establish cycle */
		if (msg_peer_stopping(hdr)) {
			rc = tipc_link_fsm_evt(l, LINK_FAILURE_EVT);
			break;
		}

		/* If this endpoint was re-created while peer was ESTABLISHING
		 * it doesn't know current session number. Force re-synch.
		 */
		if (mtyp == ACTIVATE_MSG && msg_dest_session_valid(hdr) &&
		    l->session != msg_dest_session(hdr)) {
			if (less(l->session, msg_dest_session(hdr)))
				l->session = msg_dest_session(hdr) + 1;
			break;
		}

		/* ACTIVATE_MSG serves as PEER_RESET if link is already down */
		if (mtyp == RESET_MSG || !link_is_up(l))
			rc = tipc_link_fsm_evt(l, LINK_PEER_RESET_EVT);

		/* ACTIVATE_MSG takes up link if it was already locally reset */
		if (mtyp == ACTIVATE_MSG && l->state == LINK_ESTABLISHING)
			rc = TIPC_LINK_UP_EVT;

		l->peer_session = msg_session(hdr);
		l->in_session = true;
		l->peer_bearer_id = msg_bearer_id(hdr);
		if (l->mtu > msg_max_pkt(hdr))
			l->mtu = msg_max_pkt(hdr);
		break;

	case STATE_MSG:
		l->rcv_nxt_state = msg_seqno(hdr) + 1;

		/* Update own tolerance if peer indicates a non-zero value */
		if (in_range(peers_tol, TIPC_MIN_LINK_TOL, TIPC_MAX_LINK_TOL)) {
			l->tolerance = peers_tol;
			l->bc_rcvlink->tolerance = peers_tol;
		}
		/* Update own prio if peer indicates a different value */
		if ((peers_prio != l->priority) &&
		    in_range(peers_prio, 1, TIPC_MAX_LINK_PRI)) {
			l->priority = peers_prio;
			rc = tipc_link_fsm_evt(l, LINK_FAILURE_EVT);
		}

		l->silent_intv_cnt = 0;
		l->stats.recv_states++;
		if (msg_probe(hdr))
			l->stats.recv_probes++;

		if (!link_is_up(l)) {
			if (l->state == LINK_ESTABLISHING)
				rc = TIPC_LINK_UP_EVT;
			break;
		}

		/* Receive Gap ACK blocks from peer if any */
		glen = tipc_get_gap_ack_blks(&ga, l, hdr, true);

		tipc_mon_rcv(l->net, data + glen, dlen - glen, l->addr,
			     &l->mon_state, l->bearer_id);

		/* Send NACK if peer has sent pkts we haven't received yet */
		if ((reply || msg_is_keepalive(hdr)) &&
		    more(peers_snd_nxt, rcv_nxt) &&
		    !tipc_link_is_synching(l) &&
		    skb_queue_empty(&l->deferdq))
			rcvgap = peers_snd_nxt - l->rcv_nxt;
		if (rcvgap || reply)
			tipc_link_build_proto_msg(l, STATE_MSG, 0, reply,
						  rcvgap, 0, 0, xmitq);

		released = tipc_link_advance_transmq(l, l, ack, gap, ga, xmitq,
						     &retransmitted, &rc);
		if (gap)
			l->stats.recv_nacks++;
		if (released || retransmitted)
			tipc_link_update_cwin(l, released, retransmitted);
		if (released)
			tipc_link_advance_backlog(l, xmitq);
		if (unlikely(!skb_queue_empty(&l->wakeupq)))
			link_prepare_wakeup(l);
	}
exit:
	kfree_skb(skb);
	return rc;
}

/* tipc_link_build_bc_proto_msg() - create broadcast protocol message
 */
static bool tipc_link_build_bc_proto_msg(struct tipc_link *l, bool bcast,
					 u16 peers_snd_nxt,
					 struct sk_buff_head *xmitq)
{
	struct sk_buff *skb;
	struct tipc_msg *hdr;
	struct sk_buff *dfrd_skb = skb_peek(&l->deferdq);
	u16 ack = l->rcv_nxt - 1;
	u16 gap_to = peers_snd_nxt - 1;

	skb = tipc_msg_create(BCAST_PROTOCOL, STATE_MSG, INT_H_SIZE,
			      0, l->addr, tipc_own_addr(l->net), 0, 0, 0);
	if (!skb)
		return false;
	hdr = buf_msg(skb);
	msg_set_last_bcast(hdr, l->bc_sndlink->snd_nxt - 1);
	msg_set_bcast_ack(hdr, ack);
	msg_set_bcgap_after(hdr, ack);
	if (dfrd_skb)
		gap_to = buf_seqno(dfrd_skb) - 1;
	msg_set_bcgap_to(hdr, gap_to);
	msg_set_non_seq(hdr, bcast);
	__skb_queue_tail(xmitq, skb);
	return true;
}

/* tipc_link_build_bc_init_msg() - synchronize broadcast link endpoints.
 *
 * Give a newly added peer node the sequence number where it should
 * start receiving and acking broadcast packets.
 */
static void tipc_link_build_bc_init_msg(struct tipc_link *l,
					struct sk_buff_head *xmitq)
{
	struct sk_buff_head list;

	__skb_queue_head_init(&list);
	if (!tipc_link_build_bc_proto_msg(l->bc_rcvlink, false, 0, &list))
		return;
	msg_set_bc_ack_invalid(buf_msg(skb_peek(&list)), true);
	tipc_link_xmit(l, &list, xmitq);
}

/* tipc_link_bc_init_rcv - receive initial broadcast synch data from peer
 */
void tipc_link_bc_init_rcv(struct tipc_link *l, struct tipc_msg *hdr)
{
	int mtyp = msg_type(hdr);
	u16 peers_snd_nxt = msg_bc_snd_nxt(hdr);

	if (link_is_up(l))
		return;

	if (msg_user(hdr) == BCAST_PROTOCOL) {
		l->rcv_nxt = peers_snd_nxt;
		l->state = LINK_ESTABLISHED;
		return;
	}

	if (l->peer_caps & TIPC_BCAST_SYNCH)
		return;

	if (msg_peer_node_is_up(hdr))
		return;

	/* Compatibility: accept older, less safe initial synch data */
	if ((mtyp == RESET_MSG) || (mtyp == ACTIVATE_MSG))
		l->rcv_nxt = peers_snd_nxt;
}

/* tipc_link_bc_sync_rcv - update rcv link according to peer's send state
 */
int tipc_link_bc_sync_rcv(struct tipc_link *l, struct tipc_msg *hdr,
			  struct sk_buff_head *xmitq)
{
	u16 peers_snd_nxt = msg_bc_snd_nxt(hdr);
	int rc = 0;

	if (!link_is_up(l))
		return rc;

	if (!msg_peer_node_is_up(hdr))
		return rc;

	/* Open when peer acknowledges our bcast init msg (pkt #1) */
	if (msg_ack(hdr))
		l->bc_peer_is_up = true;

	if (!l->bc_peer_is_up)
		return rc;

	/* Ignore if peers_snd_nxt goes beyond receive window */
	if (more(peers_snd_nxt, l->rcv_nxt + l->window))
		return rc;

	l->snd_nxt = peers_snd_nxt;
	if (link_bc_rcv_gap(l))
		rc |= TIPC_LINK_SND_STATE;

	/* Return now if sender supports nack via STATE messages */
	if (l->peer_caps & TIPC_BCAST_STATE_NACK)
		return rc;

	/* Otherwise, be backwards compatible */

	if (!more(peers_snd_nxt, l->rcv_nxt)) {
		l->nack_state = BC_NACK_SND_CONDITIONAL;
		return 0;
	}

	/* Don't NACK if one was recently sent or peeked */
	if (l->nack_state == BC_NACK_SND_SUPPRESS) {
		l->nack_state = BC_NACK_SND_UNCONDITIONAL;
		return 0;
	}

	/* Conditionally delay NACK sending until next synch rcv */
	if (l->nack_state == BC_NACK_SND_CONDITIONAL) {
		l->nack_state = BC_NACK_SND_UNCONDITIONAL;
		if ((peers_snd_nxt - l->rcv_nxt) < TIPC_MIN_LINK_WIN)
			return 0;
	}

	/* Send NACK now but suppress next one */
	tipc_link_build_bc_proto_msg(l, true, peers_snd_nxt, xmitq);
	l->nack_state = BC_NACK_SND_SUPPRESS;
	return 0;
}

int tipc_link_bc_ack_rcv(struct tipc_link *r, u16 acked, u16 gap,
			 struct tipc_gap_ack_blks *ga,
			 struct sk_buff_head *xmitq,
			 struct sk_buff_head *retrq)
{
	struct tipc_link *l = r->bc_sndlink;
	bool unused = false;
	int rc = 0;

	if (!link_is_up(r) || !r->bc_peer_is_up)
		return 0;

	if (gap) {
		l->stats.recv_nacks++;
		r->stats.recv_nacks++;
	}

	if (less(acked, r->acked) || (acked == r->acked && !gap && !ga))
		return 0;

	trace_tipc_link_bc_ack(r, acked, gap, &l->transmq);
	tipc_link_advance_transmq(l, r, acked, gap, ga, retrq, &unused, &rc);

	tipc_link_advance_backlog(l, xmitq);
	if (unlikely(!skb_queue_empty(&l->wakeupq)))
		link_prepare_wakeup(l);

	return rc;
}

/* tipc_link_bc_nack_rcv(): receive broadcast nack message
 * This function is here for backwards compatibility, since
 * no BCAST_PROTOCOL/STATE messages occur from TIPC v2.5.
 */
int tipc_link_bc_nack_rcv(struct tipc_link *l, struct sk_buff *skb,
			  struct sk_buff_head *xmitq)
{
	struct tipc_msg *hdr = buf_msg(skb);
	u32 dnode = msg_destnode(hdr);
	int mtyp = msg_type(hdr);
	u16 acked = msg_bcast_ack(hdr);
	u16 from = acked + 1;
	u16 to = msg_bcgap_to(hdr);
	u16 peers_snd_nxt = to + 1;
	int rc = 0;

	kfree_skb(skb);

	if (!tipc_link_is_up(l) || !l->bc_peer_is_up)
		return 0;

	if (mtyp != STATE_MSG)
		return 0;

	if (dnode == tipc_own_addr(l->net)) {
		rc = tipc_link_bc_ack_rcv(l, acked, to - acked, NULL, xmitq,
					  xmitq);
		l->stats.recv_nacks++;
		return rc;
	}

	/* Msg for other node => suppress own NACK at next sync if applicable */
	if (more(peers_snd_nxt, l->rcv_nxt) && !less(l->rcv_nxt, from))
		l->nack_state = BC_NACK_SND_SUPPRESS;

	return 0;
}

void tipc_link_set_queue_limits(struct tipc_link *l, u32 min_win, u32 max_win)
{
	int max_bulk = TIPC_MAX_PUBL / (l->mtu / ITEM_SIZE);

	l->min_win = min_win;
	l->ssthresh = max_win;
	l->max_win = max_win;
	l->window = min_win;
	l->backlog[TIPC_LOW_IMPORTANCE].limit      = min_win * 2;
	l->backlog[TIPC_MEDIUM_IMPORTANCE].limit   = min_win * 4;
	l->backlog[TIPC_HIGH_IMPORTANCE].limit     = min_win * 6;
	l->backlog[TIPC_CRITICAL_IMPORTANCE].limit = min_win * 8;
	l->backlog[TIPC_SYSTEM_IMPORTANCE].limit   = max_bulk;
}

/**
 * tipc_link_reset_stats - reset link statistics
 * @l: pointer to link
 */
void tipc_link_reset_stats(struct tipc_link *l)
{
	memset(&l->stats, 0, sizeof(l->stats));
}

static void link_print(struct tipc_link *l, const char *str)
{
	struct sk_buff *hskb = skb_peek(&l->transmq);
	u16 head = hskb ? msg_seqno(buf_msg(hskb)) : l->snd_nxt - 1;
	u16 tail = l->snd_nxt - 1;

	pr_info("%s Link <%s> state %x\n", str, l->name, l->state);
	pr_info("XMTQ: %u [%u-%u], BKLGQ: %u, SNDNX: %u, RCVNX: %u\n",
		skb_queue_len(&l->transmq), head, tail,
		skb_queue_len(&l->backlogq), l->snd_nxt, l->rcv_nxt);
}

/* Parse and validate nested (link) properties valid for media, bearer and link
 */
int tipc_nl_parse_link_prop(struct nlattr *prop, struct nlattr *props[])
{
	int err;

	err = nla_parse_nested_deprecated(props, TIPC_NLA_PROP_MAX, prop,
					  tipc_nl_prop_policy, NULL);
	if (err)
		return err;

	if (props[TIPC_NLA_PROP_PRIO]) {
		u32 prio;

		prio = nla_get_u32(props[TIPC_NLA_PROP_PRIO]);
		if (prio > TIPC_MAX_LINK_PRI)
			return -EINVAL;
	}

	if (props[TIPC_NLA_PROP_TOL]) {
		u32 tol;

		tol = nla_get_u32(props[TIPC_NLA_PROP_TOL]);
		if ((tol < TIPC_MIN_LINK_TOL) || (tol > TIPC_MAX_LINK_TOL))
			return -EINVAL;
	}

	if (props[TIPC_NLA_PROP_WIN]) {
		u32 max_win;

		max_win = nla_get_u32(props[TIPC_NLA_PROP_WIN]);
		if (max_win < TIPC_DEF_LINK_WIN || max_win > TIPC_MAX_LINK_WIN)
			return -EINVAL;
	}

	return 0;
}

static int __tipc_nl_add_stats(struct sk_buff *skb, struct tipc_stats *s)
{
	int i;
	struct nlattr *stats;

	struct nla_map {
		u32 key;
		u32 val;
	};

	struct nla_map map[] = {
		{TIPC_NLA_STATS_RX_INFO, 0},
		{TIPC_NLA_STATS_RX_FRAGMENTS, s->recv_fragments},
		{TIPC_NLA_STATS_RX_FRAGMENTED, s->recv_fragmented},
		{TIPC_NLA_STATS_RX_BUNDLES, s->recv_bundles},
		{TIPC_NLA_STATS_RX_BUNDLED, s->recv_bundled},
		{TIPC_NLA_STATS_TX_INFO, 0},
		{TIPC_NLA_STATS_TX_FRAGMENTS, s->sent_fragments},
		{TIPC_NLA_STATS_TX_FRAGMENTED, s->sent_fragmented},
		{TIPC_NLA_STATS_TX_BUNDLES, s->sent_bundles},
		{TIPC_NLA_STATS_TX_BUNDLED, s->sent_bundled},
		{TIPC_NLA_STATS_MSG_PROF_TOT, (s->msg_length_counts) ?
			s->msg_length_counts : 1},
		{TIPC_NLA_STATS_MSG_LEN_CNT, s->msg_length_counts},
		{TIPC_NLA_STATS_MSG_LEN_TOT, s->msg_lengths_total},
		{TIPC_NLA_STATS_MSG_LEN_P0, s->msg_length_profile[0]},
		{TIPC_NLA_STATS_MSG_LEN_P1, s->msg_length_profile[1]},
		{TIPC_NLA_STATS_MSG_LEN_P2, s->msg_length_profile[2]},
		{TIPC_NLA_STATS_MSG_LEN_P3, s->msg_length_profile[3]},
		{TIPC_NLA_STATS_MSG_LEN_P4, s->msg_length_profile[4]},
		{TIPC_NLA_STATS_MSG_LEN_P5, s->msg_length_profile[5]},
		{TIPC_NLA_STATS_MSG_LEN_P6, s->msg_length_profile[6]},
		{TIPC_NLA_STATS_RX_STATES, s->recv_states},
		{TIPC_NLA_STATS_RX_PROBES, s->recv_probes},
		{TIPC_NLA_STATS_RX_NACKS, s->recv_nacks},
		{TIPC_NLA_STATS_RX_DEFERRED, s->deferred_recv},
		{TIPC_NLA_STATS_TX_STATES, s->sent_states},
		{TIPC_NLA_STATS_TX_PROBES, s->sent_probes},
		{TIPC_NLA_STATS_TX_NACKS, s->sent_nacks},
		{TIPC_NLA_STATS_TX_ACKS, s->sent_acks},
		{TIPC_NLA_STATS_RETRANSMITTED, s->retransmitted},
		{TIPC_NLA_STATS_DUPLICATES, s->duplicates},
		{TIPC_NLA_STATS_LINK_CONGS, s->link_congs},
		{TIPC_NLA_STATS_MAX_QUEUE, s->max_queue_sz},
		{TIPC_NLA_STATS_AVG_QUEUE, s->queue_sz_counts ?
			(s->accu_queue_sz / s->queue_sz_counts) : 0}
	};

	stats = nla_nest_start_noflag(skb, TIPC_NLA_LINK_STATS);
	if (!stats)
		return -EMSGSIZE;

	for (i = 0; i <  ARRAY_SIZE(map); i++)
		if (nla_put_u32(skb, map[i].key, map[i].val))
			goto msg_full;

	nla_nest_end(skb, stats);

	return 0;
msg_full:
	nla_nest_cancel(skb, stats);

	return -EMSGSIZE;
}

/* Caller should hold appropriate locks to protect the link */
int __tipc_nl_add_link(struct net *net, struct tipc_nl_msg *msg,
		       struct tipc_link *link, int nlflags)
{
	u32 self = tipc_own_addr(net);
	struct nlattr *attrs;
	struct nlattr *prop;
	void *hdr;
	int err;

	hdr = genlmsg_put(msg->skb, msg->portid, msg->seq, &tipc_genl_family,
			  nlflags, TIPC_NL_LINK_GET);
	if (!hdr)
		return -EMSGSIZE;

	attrs = nla_nest_start_noflag(msg->skb, TIPC_NLA_LINK);
	if (!attrs)
		goto msg_full;

	if (nla_put_string(msg->skb, TIPC_NLA_LINK_NAME, link->name))
		goto attr_msg_full;
	if (nla_put_u32(msg->skb, TIPC_NLA_LINK_DEST, tipc_cluster_mask(self)))
		goto attr_msg_full;
	if (nla_put_u32(msg->skb, TIPC_NLA_LINK_MTU, link->mtu))
		goto attr_msg_full;
	if (nla_put_u32(msg->skb, TIPC_NLA_LINK_RX, link->stats.recv_pkts))
		goto attr_msg_full;
	if (nla_put_u32(msg->skb, TIPC_NLA_LINK_TX, link->stats.sent_pkts))
		goto attr_msg_full;

	if (tipc_link_is_up(link))
		if (nla_put_flag(msg->skb, TIPC_NLA_LINK_UP))
			goto attr_msg_full;
	if (link->active)
		if (nla_put_flag(msg->skb, TIPC_NLA_LINK_ACTIVE))
			goto attr_msg_full;

	prop = nla_nest_start_noflag(msg->skb, TIPC_NLA_LINK_PROP);
	if (!prop)
		goto attr_msg_full;
	if (nla_put_u32(msg->skb, TIPC_NLA_PROP_PRIO, link->priority))
		goto prop_msg_full;
	if (nla_put_u32(msg->skb, TIPC_NLA_PROP_TOL, link->tolerance))
		goto prop_msg_full;
	if (nla_put_u32(msg->skb, TIPC_NLA_PROP_WIN,
			link->window))
		goto prop_msg_full;
	if (nla_put_u32(msg->skb, TIPC_NLA_PROP_PRIO, link->priority))
		goto prop_msg_full;
	nla_nest_end(msg->skb, prop);

	err = __tipc_nl_add_stats(msg->skb, &link->stats);
	if (err)
		goto attr_msg_full;

	nla_nest_end(msg->skb, attrs);
	genlmsg_end(msg->skb, hdr);

	return 0;

prop_msg_full:
	nla_nest_cancel(msg->skb, prop);
attr_msg_full:
	nla_nest_cancel(msg->skb, attrs);
msg_full:
	genlmsg_cancel(msg->skb, hdr);

	return -EMSGSIZE;
}

static int __tipc_nl_add_bc_link_stat(struct sk_buff *skb,
				      struct tipc_stats *stats)
{
	int i;
	struct nlattr *nest;

	struct nla_map {
		__u32 key;
		__u32 val;
	};

	struct nla_map map[] = {
		{TIPC_NLA_STATS_RX_INFO, stats->recv_pkts},
		{TIPC_NLA_STATS_RX_FRAGMENTS, stats->recv_fragments},
		{TIPC_NLA_STATS_RX_FRAGMENTED, stats->recv_fragmented},
		{TIPC_NLA_STATS_RX_BUNDLES, stats->recv_bundles},
		{TIPC_NLA_STATS_RX_BUNDLED, stats->recv_bundled},
		{TIPC_NLA_STATS_TX_INFO, stats->sent_pkts},
		{TIPC_NLA_STATS_TX_FRAGMENTS, stats->sent_fragments},
		{TIPC_NLA_STATS_TX_FRAGMENTED, stats->sent_fragmented},
		{TIPC_NLA_STATS_TX_BUNDLES, stats->sent_bundles},
		{TIPC_NLA_STATS_TX_BUNDLED, stats->sent_bundled},
		{TIPC_NLA_STATS_RX_NACKS, stats->recv_nacks},
		{TIPC_NLA_STATS_RX_DEFERRED, stats->deferred_recv},
		{TIPC_NLA_STATS_TX_NACKS, stats->sent_nacks},
		{TIPC_NLA_STATS_TX_ACKS, stats->sent_acks},
		{TIPC_NLA_STATS_RETRANSMITTED, stats->retransmitted},
		{TIPC_NLA_STATS_DUPLICATES, stats->duplicates},
		{TIPC_NLA_STATS_LINK_CONGS, stats->link_congs},
		{TIPC_NLA_STATS_MAX_QUEUE, stats->max_queue_sz},
		{TIPC_NLA_STATS_AVG_QUEUE, stats->queue_sz_counts ?
			(stats->accu_queue_sz / stats->queue_sz_counts) : 0}
	};

	nest = nla_nest_start_noflag(skb, TIPC_NLA_LINK_STATS);
	if (!nest)
		return -EMSGSIZE;

	for (i = 0; i <  ARRAY_SIZE(map); i++)
		if (nla_put_u32(skb, map[i].key, map[i].val))
			goto msg_full;

	nla_nest_end(skb, nest);

	return 0;
msg_full:
	nla_nest_cancel(skb, nest);

	return -EMSGSIZE;
}

int tipc_nl_add_bc_link(struct net *net, struct tipc_nl_msg *msg,
			struct tipc_link *bcl)
{
	int err;
	void *hdr;
	struct nlattr *attrs;
	struct nlattr *prop;
	u32 bc_mode = tipc_bcast_get_mode(net);
	u32 bc_ratio = tipc_bcast_get_broadcast_ratio(net);

	if (!bcl)
		return 0;

	tipc_bcast_lock(net);

	hdr = genlmsg_put(msg->skb, msg->portid, msg->seq, &tipc_genl_family,
			  NLM_F_MULTI, TIPC_NL_LINK_GET);
	if (!hdr) {
		tipc_bcast_unlock(net);
		return -EMSGSIZE;
	}

	attrs = nla_nest_start_noflag(msg->skb, TIPC_NLA_LINK);
	if (!attrs)
		goto msg_full;

	/* The broadcast link is always up */
	if (nla_put_flag(msg->skb, TIPC_NLA_LINK_UP))
		goto attr_msg_full;

	if (nla_put_flag(msg->skb, TIPC_NLA_LINK_BROADCAST))
		goto attr_msg_full;
	if (nla_put_string(msg->skb, TIPC_NLA_LINK_NAME, bcl->name))
		goto attr_msg_full;
	if (nla_put_u32(msg->skb, TIPC_NLA_LINK_RX, 0))
		goto attr_msg_full;
	if (nla_put_u32(msg->skb, TIPC_NLA_LINK_TX, 0))
		goto attr_msg_full;

	prop = nla_nest_start_noflag(msg->skb, TIPC_NLA_LINK_PROP);
	if (!prop)
		goto attr_msg_full;
	if (nla_put_u32(msg->skb, TIPC_NLA_PROP_WIN, bcl->max_win))
		goto prop_msg_full;
	if (nla_put_u32(msg->skb, TIPC_NLA_PROP_BROADCAST, bc_mode))
		goto prop_msg_full;
	if (bc_mode & BCLINK_MODE_SEL)
		if (nla_put_u32(msg->skb, TIPC_NLA_PROP_BROADCAST_RATIO,
				bc_ratio))
			goto prop_msg_full;
	nla_nest_end(msg->skb, prop);

	err = __tipc_nl_add_bc_link_stat(msg->skb, &bcl->stats);
	if (err)
		goto attr_msg_full;

	tipc_bcast_unlock(net);
	nla_nest_end(msg->skb, attrs);
	genlmsg_end(msg->skb, hdr);

	return 0;

prop_msg_full:
	nla_nest_cancel(msg->skb, prop);
attr_msg_full:
	nla_nest_cancel(msg->skb, attrs);
msg_full:
	tipc_bcast_unlock(net);
	genlmsg_cancel(msg->skb, hdr);

	return -EMSGSIZE;
}

void tipc_link_set_tolerance(struct tipc_link *l, u32 tol,
			     struct sk_buff_head *xmitq)
{
	l->tolerance = tol;
	if (l->bc_rcvlink)
		l->bc_rcvlink->tolerance = tol;
	if (link_is_up(l))
		tipc_link_build_proto_msg(l, STATE_MSG, 0, 0, 0, tol, 0, xmitq);
}

void tipc_link_set_prio(struct tipc_link *l, u32 prio,
			struct sk_buff_head *xmitq)
{
	l->priority = prio;
	tipc_link_build_proto_msg(l, STATE_MSG, 0, 0, 0, 0, prio, xmitq);
}

void tipc_link_set_abort_limit(struct tipc_link *l, u32 limit)
{
	l->abort_limit = limit;
}

/**
 * tipc_link_dump - dump TIPC link data
 * @l: tipc link to be dumped
 * @dqueues: bitmask to decide if any link queue to be dumped?
 *           - TIPC_DUMP_NONE: don't dump link queues
 *           - TIPC_DUMP_TRANSMQ: dump link transmq queue
 *           - TIPC_DUMP_BACKLOGQ: dump link backlog queue
 *           - TIPC_DUMP_DEFERDQ: dump link deferd queue
 *           - TIPC_DUMP_INPUTQ: dump link input queue
 *           - TIPC_DUMP_WAKEUP: dump link wakeup queue
 *           - TIPC_DUMP_ALL: dump all the link queues above
 * @buf: returned buffer of dump data in format
 */
int tipc_link_dump(struct tipc_link *l, u16 dqueues, char *buf)
{
	int i = 0;
	size_t sz = (dqueues) ? LINK_LMAX : LINK_LMIN;
	struct sk_buff_head *list;
	struct sk_buff *hskb, *tskb;
	u32 len;

	if (!l) {
		i += scnprintf(buf, sz, "link data: (null)\n");
		return i;
	}

	i += scnprintf(buf, sz, "link data: %x", l->addr);
	i += scnprintf(buf + i, sz - i, " %x", l->state);
	i += scnprintf(buf + i, sz - i, " %u", l->in_session);
	i += scnprintf(buf + i, sz - i, " %u", l->session);
	i += scnprintf(buf + i, sz - i, " %u", l->peer_session);
	i += scnprintf(buf + i, sz - i, " %u", l->snd_nxt);
	i += scnprintf(buf + i, sz - i, " %u", l->rcv_nxt);
	i += scnprintf(buf + i, sz - i, " %u", l->snd_nxt_state);
	i += scnprintf(buf + i, sz - i, " %u", l->rcv_nxt_state);
	i += scnprintf(buf + i, sz - i, " %x", l->peer_caps);
	i += scnprintf(buf + i, sz - i, " %u", l->silent_intv_cnt);
	i += scnprintf(buf + i, sz - i, " %u", l->rst_cnt);
	i += scnprintf(buf + i, sz - i, " %u", 0);
	i += scnprintf(buf + i, sz - i, " %u", 0);
	i += scnprintf(buf + i, sz - i, " %u", l->acked);

	list = &l->transmq;
	len = skb_queue_len(list);
	hskb = skb_peek(list);
	tskb = skb_peek_tail(list);
	i += scnprintf(buf + i, sz - i, " | %u %u %u", len,
		       (hskb) ? msg_seqno(buf_msg(hskb)) : 0,
		       (tskb) ? msg_seqno(buf_msg(tskb)) : 0);

	list = &l->deferdq;
	len = skb_queue_len(list);
	hskb = skb_peek(list);
	tskb = skb_peek_tail(list);
	i += scnprintf(buf + i, sz - i, " | %u %u %u", len,
		       (hskb) ? msg_seqno(buf_msg(hskb)) : 0,
		       (tskb) ? msg_seqno(buf_msg(tskb)) : 0);

	list = &l->backlogq;
	len = skb_queue_len(list);
	hskb = skb_peek(list);
	tskb = skb_peek_tail(list);
	i += scnprintf(buf + i, sz - i, " | %u %u %u", len,
		       (hskb) ? msg_seqno(buf_msg(hskb)) : 0,
		       (tskb) ? msg_seqno(buf_msg(tskb)) : 0);

	list = l->inputq;
	len = skb_queue_len(list);
	hskb = skb_peek(list);
	tskb = skb_peek_tail(list);
	i += scnprintf(buf + i, sz - i, " | %u %u %u\n", len,
		       (hskb) ? msg_seqno(buf_msg(hskb)) : 0,
		       (tskb) ? msg_seqno(buf_msg(tskb)) : 0);

	if (dqueues & TIPC_DUMP_TRANSMQ) {
		i += scnprintf(buf + i, sz - i, "transmq: ");
		i += tipc_list_dump(&l->transmq, false, buf + i);
	}
	if (dqueues & TIPC_DUMP_BACKLOGQ) {
		i += scnprintf(buf + i, sz - i,
			       "backlogq: <%u %u %u %u %u>, ",
			       l->backlog[TIPC_LOW_IMPORTANCE].len,
			       l->backlog[TIPC_MEDIUM_IMPORTANCE].len,
			       l->backlog[TIPC_HIGH_IMPORTANCE].len,
			       l->backlog[TIPC_CRITICAL_IMPORTANCE].len,
			       l->backlog[TIPC_SYSTEM_IMPORTANCE].len);
		i += tipc_list_dump(&l->backlogq, false, buf + i);
	}
	if (dqueues & TIPC_DUMP_DEFERDQ) {
		i += scnprintf(buf + i, sz - i, "deferdq: ");
		i += tipc_list_dump(&l->deferdq, false, buf + i);
	}
	if (dqueues & TIPC_DUMP_INPUTQ) {
		i += scnprintf(buf + i, sz - i, "inputq: ");
		i += tipc_list_dump(l->inputq, false, buf + i);
	}
	if (dqueues & TIPC_DUMP_WAKEUP) {
		i += scnprintf(buf + i, sz - i, "wakeup: ");
		i += tipc_list_dump(&l->wakeupq, false, buf + i);
	}

	return i;
}