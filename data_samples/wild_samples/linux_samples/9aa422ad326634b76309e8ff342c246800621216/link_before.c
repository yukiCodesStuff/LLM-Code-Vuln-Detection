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