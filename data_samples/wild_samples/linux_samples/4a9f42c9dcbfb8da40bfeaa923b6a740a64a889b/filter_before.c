	switch (func_id) {
	case BPF_FUNC_skb_store_bytes:
		return &bpf_skb_store_bytes_proto;
	case BPF_FUNC_skb_load_bytes:
		return &bpf_skb_load_bytes_proto;
	case BPF_FUNC_skb_pull_data:
		return &sk_skb_pull_data_proto;
	case BPF_FUNC_skb_change_tail:
		return &sk_skb_change_tail_proto;
	case BPF_FUNC_skb_change_head:
		return &sk_skb_change_head_proto;
	case BPF_FUNC_get_socket_cookie:
		return &bpf_get_socket_cookie_proto;
	case BPF_FUNC_get_socket_uid:
		return &bpf_get_socket_uid_proto;
	case BPF_FUNC_sk_redirect_map:
		return &bpf_sk_redirect_map_proto;
	case BPF_FUNC_sk_redirect_hash:
		return &bpf_sk_redirect_hash_proto;
	case BPF_FUNC_get_local_storage:
		return &bpf_get_local_storage_proto;
	default:
		return bpf_base_func_proto(func_id);
	}
}

static const struct bpf_func_proto *
lwt_out_func_proto(enum bpf_func_id func_id, const struct bpf_prog *prog)
{
	switch (func_id) {
	case BPF_FUNC_skb_load_bytes:
		return &bpf_skb_load_bytes_proto;
	case BPF_FUNC_skb_pull_data:
		return &bpf_skb_pull_data_proto;
	case BPF_FUNC_csum_diff:
		return &bpf_csum_diff_proto;
	case BPF_FUNC_get_cgroup_classid:
		return &bpf_get_cgroup_classid_proto;
	case BPF_FUNC_get_route_realm:
		return &bpf_get_route_realm_proto;
	case BPF_FUNC_get_hash_recalc:
		return &bpf_get_hash_recalc_proto;
	case BPF_FUNC_perf_event_output:
		return &bpf_skb_event_output_proto;
	case BPF_FUNC_get_smp_processor_id:
		return &bpf_get_smp_processor_id_proto;
	case BPF_FUNC_skb_under_cgroup:
		return &bpf_skb_under_cgroup_proto;
	default:
		return bpf_base_func_proto(func_id);
	}
	case bpf_ctx_range(struct __sk_buff, data_end):
		if (size != size_default)
			return false;
		break;
	default:
		/* Only narrow read access allowed for now. */
		if (type == BPF_WRITE) {
			if (size != size_default)
				return false;
		} else {
			bpf_ctx_record_field_size(info, size_default);
			if (!bpf_ctx_narrow_access_ok(off, size, size_default))
				return false;
		}
	}
	case bpf_ctx_range(struct __sk_buff, data_meta):
	case bpf_ctx_range(struct __sk_buff, data_end):
	case bpf_ctx_range_till(struct __sk_buff, family, local_port):
		return false;
	}

	if (type == BPF_WRITE) {
		switch (off) {
		case bpf_ctx_range_till(struct __sk_buff, cb[0], cb[4]):
			break;
		default:
			return false;
		}
	case bpf_ctx_range_till(struct __sk_buff, family, local_port):
	case bpf_ctx_range(struct __sk_buff, data_meta):
		return false;
	}

	if (type == BPF_WRITE) {
		switch (off) {
		case bpf_ctx_range(struct __sk_buff, mark):
		case bpf_ctx_range(struct __sk_buff, priority):
		case bpf_ctx_range_till(struct __sk_buff, cb[0], cb[4]):
			break;
		default:
			return false;
		}
	case bpf_ctx_range(struct __sk_buff, data_end):
		info->reg_type = PTR_TO_PACKET_END;
		break;
	case bpf_ctx_range_till(struct __sk_buff, family, local_port):
		return false;
	}

	return bpf_skb_is_valid_access(off, size, type, prog, info);
}

static bool __is_valid_xdp_access(int off, int size)
{
	if (off < 0 || off >= sizeof(struct xdp_md))
		return false;
	if (off % size != 0)
		return false;
	if (size != sizeof(__u32))
		return false;

	return true;
}

static bool xdp_is_valid_access(int off, int size,
				enum bpf_access_type type,
				const struct bpf_prog *prog,
				struct bpf_insn_access_aux *info)
{
	if (type == BPF_WRITE) {
		if (bpf_prog_is_dev_bound(prog->aux)) {
			switch (off) {
			case offsetof(struct xdp_md, rx_queue_index):
				return __is_valid_xdp_access(off, size);
			}
		}
	case bpf_ctx_range(struct __sk_buff, tc_classid):
	case bpf_ctx_range(struct __sk_buff, data_meta):
		return false;
	}

	if (type == BPF_WRITE) {
		switch (off) {
		case bpf_ctx_range(struct __sk_buff, tc_index):
		case bpf_ctx_range(struct __sk_buff, priority):
			break;
		default:
			return false;
		}
	case offsetof(struct sk_msg_md, data_end):
		info->reg_type = PTR_TO_PACKET_END;
		if (size != sizeof(__u64))
			return false;
		break;
	default:
		if (size != sizeof(__u32))
			return false;
	}

	if (off < 0 || off >= sizeof(struct sk_msg_md))
		return false;
	if (off % size != 0)
		return false;

	return true;
}

static u32 bpf_convert_ctx_access(enum bpf_access_type type,
				  const struct bpf_insn *si,
				  struct bpf_insn *insn_buf,
				  struct bpf_prog *prog, u32 *target_size)
{
	struct bpf_insn *insn = insn_buf;
	int off;

	switch (si->off) {
	case offsetof(struct __sk_buff, len):
		*insn++ = BPF_LDX_MEM(BPF_W, si->dst_reg, si->src_reg,
				      bpf_target_off(struct sk_buff, len, 4,
						     target_size));
		break;

	case offsetof(struct __sk_buff, protocol):
		*insn++ = BPF_LDX_MEM(BPF_H, si->dst_reg, si->src_reg,
				      bpf_target_off(struct sk_buff, protocol, 2,
						     target_size));
		break;

	case offsetof(struct __sk_buff, vlan_proto):
		*insn++ = BPF_LDX_MEM(BPF_H, si->dst_reg, si->src_reg,
				      bpf_target_off(struct sk_buff, vlan_proto, 2,
						     target_size));
		break;

	case offsetof(struct __sk_buff, priority):
		if (type == BPF_WRITE)
			*insn++ = BPF_STX_MEM(BPF_W, si->dst_reg, si->src_reg,
					      bpf_target_off(struct sk_buff, priority, 4,
							     target_size));
		else
			*insn++ = BPF_LDX_MEM(BPF_W, si->dst_reg, si->src_reg,
					      bpf_target_off(struct sk_buff, priority, 4,
							     target_size));
		break;

	case offsetof(struct __sk_buff, ingress_ifindex):
		*insn++ = BPF_LDX_MEM(BPF_W, si->dst_reg, si->src_reg,
				      bpf_target_off(struct sk_buff, skb_iif, 4,
						     target_size));
		break;

	case offsetof(struct __sk_buff, ifindex):
		*insn++ = BPF_LDX_MEM(BPF_FIELD_SIZEOF(struct sk_buff, dev),
				      si->dst_reg, si->src_reg,
				      offsetof(struct sk_buff, dev));
		*insn++ = BPF_JMP_IMM(BPF_JEQ, si->dst_reg, 0, 1);
		*insn++ = BPF_LDX_MEM(BPF_W, si->dst_reg, si->dst_reg,
				      bpf_target_off(struct net_device, ifindex, 4,
						     target_size));
		break;

	case offsetof(struct __sk_buff, hash):
		*insn++ = BPF_LDX_MEM(BPF_W, si->dst_reg, si->src_reg,
				      bpf_target_off(struct sk_buff, hash, 4,
						     target_size));
		break;

	case offsetof(struct __sk_buff, mark):
		if (type == BPF_WRITE)
			*insn++ = BPF_STX_MEM(BPF_W, si->dst_reg, si->src_reg,
					      bpf_target_off(struct sk_buff, mark, 4,
							     target_size));
		else
			*insn++ = BPF_LDX_MEM(BPF_W, si->dst_reg, si->src_reg,
					      bpf_target_off(struct sk_buff, mark, 4,
							     target_size));
		break;

	case offsetof(struct __sk_buff, pkt_type):
		*target_size = 1;
		*insn++ = BPF_LDX_MEM(BPF_B, si->dst_reg, si->src_reg,
				      PKT_TYPE_OFFSET());
		*insn++ = BPF_ALU32_IMM(BPF_AND, si->dst_reg, PKT_TYPE_MAX);
#ifdef __BIG_ENDIAN_BITFIELD
		*insn++ = BPF_ALU32_IMM(BPF_RSH, si->dst_reg, 5);
#endif
		break;

	case offsetof(struct __sk_buff, queue_mapping):
		*insn++ = BPF_LDX_MEM(BPF_H, si->dst_reg, si->src_reg,
				      bpf_target_off(struct sk_buff, queue_mapping, 2,
						     target_size));
		break;

	case offsetof(struct __sk_buff, vlan_present):
	case offsetof(struct __sk_buff, vlan_tci):
		BUILD_BUG_ON(VLAN_TAG_PRESENT != 0x1000);

		*insn++ = BPF_LDX_MEM(BPF_H, si->dst_reg, si->src_reg,
				      bpf_target_off(struct sk_buff, vlan_tci, 2,
						     target_size));
		if (si->off == offsetof(struct __sk_buff, vlan_tci)) {
			*insn++ = BPF_ALU32_IMM(BPF_AND, si->dst_reg,
						~VLAN_TAG_PRESENT);
		} else {
			*insn++ = BPF_ALU32_IMM(BPF_RSH, si->dst_reg, 12);
			*insn++ = BPF_ALU32_IMM(BPF_AND, si->dst_reg, 1);
		}
		break;

	case offsetof(struct __sk_buff, cb[0]) ...
	     offsetofend(struct __sk_buff, cb[4]) - 1:
		BUILD_BUG_ON(FIELD_SIZEOF(struct qdisc_skb_cb, data) < 20);
		BUILD_BUG_ON((offsetof(struct sk_buff, cb) +
			      offsetof(struct qdisc_skb_cb, data)) %
			     sizeof(__u64));

		prog->cb_access = 1;
		off  = si->off;
		off -= offsetof(struct __sk_buff, cb[0]);
		off += offsetof(struct sk_buff, cb);
		off += offsetof(struct qdisc_skb_cb, data);
		if (type == BPF_WRITE)
			*insn++ = BPF_STX_MEM(BPF_SIZE(si->code), si->dst_reg,
					      si->src_reg, off);
		else
			*insn++ = BPF_LDX_MEM(BPF_SIZE(si->code), si->dst_reg,
					      si->src_reg, off);
		break;

	case offsetof(struct __sk_buff, tc_classid):
		BUILD_BUG_ON(FIELD_SIZEOF(struct qdisc_skb_cb, tc_classid) != 2);

		off  = si->off;
		off -= offsetof(struct __sk_buff, tc_classid);
		off += offsetof(struct sk_buff, cb);
		off += offsetof(struct qdisc_skb_cb, tc_classid);
		*target_size = 2;
		if (type == BPF_WRITE)
			*insn++ = BPF_STX_MEM(BPF_H, si->dst_reg,
					      si->src_reg, off);
		else
			*insn++ = BPF_LDX_MEM(BPF_H, si->dst_reg,
					      si->src_reg, off);
		break;

	case offsetof(struct __sk_buff, data):
		*insn++ = BPF_LDX_MEM(BPF_FIELD_SIZEOF(struct sk_buff, data),
				      si->dst_reg, si->src_reg,
				      offsetof(struct sk_buff, data));
		break;

	case offsetof(struct __sk_buff, data_meta):
		off  = si->off;
		off -= offsetof(struct __sk_buff, data_meta);
		off += offsetof(struct sk_buff, cb);
		off += offsetof(struct bpf_skb_data_end, data_meta);
		*insn++ = BPF_LDX_MEM(BPF_SIZEOF(void *), si->dst_reg,
				      si->src_reg, off);
		break;

	case offsetof(struct __sk_buff, data_end):
		off  = si->off;
		off -= offsetof(struct __sk_buff, data_end);
		off += offsetof(struct sk_buff, cb);
		off += offsetof(struct bpf_skb_data_end, data_end);
		*insn++ = BPF_LDX_MEM(BPF_SIZEOF(void *), si->dst_reg,
				      si->src_reg, off);
		break;

	case offsetof(struct __sk_buff, tc_index):
#ifdef CONFIG_NET_SCHED
		if (type == BPF_WRITE)
			*insn++ = BPF_STX_MEM(BPF_H, si->dst_reg, si->src_reg,
					      bpf_target_off(struct sk_buff, tc_index, 2,
							     target_size));
		else
			*insn++ = BPF_LDX_MEM(BPF_H, si->dst_reg, si->src_reg,
					      bpf_target_off(struct sk_buff, tc_index, 2,
							     target_size));
#else
		*target_size = 2;
		if (type == BPF_WRITE)
			*insn++ = BPF_MOV64_REG(si->dst_reg, si->dst_reg);
		else
			*insn++ = BPF_MOV64_IMM(si->dst_reg, 0);
#endif
		break;

	case offsetof(struct __sk_buff, napi_id):
#if defined(CONFIG_NET_RX_BUSY_POLL)
		*insn++ = BPF_LDX_MEM(BPF_W, si->dst_reg, si->src_reg,
				      bpf_target_off(struct sk_buff, napi_id, 4,
						     target_size));
		*insn++ = BPF_JMP_IMM(BPF_JGE, si->dst_reg, MIN_NAPI_ID, 1);
		*insn++ = BPF_MOV64_IMM(si->dst_reg, 0);
#else
		*target_size = 4;
		*insn++ = BPF_MOV64_IMM(si->dst_reg, 0);
#endif
		break;
	case offsetof(struct __sk_buff, family):
		BUILD_BUG_ON(FIELD_SIZEOF(struct sock_common, skc_family) != 2);

		*insn++ = BPF_LDX_MEM(BPF_FIELD_SIZEOF(struct sk_buff, sk),
				      si->dst_reg, si->src_reg,
				      offsetof(struct sk_buff, sk));
		*insn++ = BPF_LDX_MEM(BPF_H, si->dst_reg, si->dst_reg,
				      bpf_target_off(struct sock_common,
						     skc_family,
						     2, target_size));
		break;
	case offsetof(struct __sk_buff, remote_ip4):
		BUILD_BUG_ON(FIELD_SIZEOF(struct sock_common, skc_daddr) != 4);

		*insn++ = BPF_LDX_MEM(BPF_FIELD_SIZEOF(struct sk_buff, sk),
				      si->dst_reg, si->src_reg,
				      offsetof(struct sk_buff, sk));
		*insn++ = BPF_LDX_MEM(BPF_W, si->dst_reg, si->dst_reg,
				      bpf_target_off(struct sock_common,
						     skc_daddr,
						     4, target_size));
		break;
	case offsetof(struct __sk_buff, local_ip4):
		BUILD_BUG_ON(FIELD_SIZEOF(struct sock_common,
					  skc_rcv_saddr) != 4);

		*insn++ = BPF_LDX_MEM(BPF_FIELD_SIZEOF(struct sk_buff, sk),
				      si->dst_reg, si->src_reg,
				      offsetof(struct sk_buff, sk));
		*insn++ = BPF_LDX_MEM(BPF_W, si->dst_reg, si->dst_reg,
				      bpf_target_off(struct sock_common,
						     skc_rcv_saddr,
						     4, target_size));
		break;
	case offsetof(struct __sk_buff, remote_ip6[0]) ...
	     offsetof(struct __sk_buff, remote_ip6[3]):
#if IS_ENABLED(CONFIG_IPV6)
		BUILD_BUG_ON(FIELD_SIZEOF(struct sock_common,
					  skc_v6_daddr.s6_addr32[0]) != 4);

		off = si->off;
		off -= offsetof(struct __sk_buff, remote_ip6[0]);

		*insn++ = BPF_LDX_MEM(BPF_FIELD_SIZEOF(struct sk_buff, sk),
				      si->dst_reg, si->src_reg,
				      offsetof(struct sk_buff, sk));
		*insn++ = BPF_LDX_MEM(BPF_W, si->dst_reg, si->dst_reg,
				      offsetof(struct sock_common,
					       skc_v6_daddr.s6_addr32[0]) +
				      off);
#else
		*insn++ = BPF_MOV32_IMM(si->dst_reg, 0);
#endif
		break;
	case offsetof(struct __sk_buff, local_ip6[0]) ...
	     offsetof(struct __sk_buff, local_ip6[3]):
#if IS_ENABLED(CONFIG_IPV6)
		BUILD_BUG_ON(FIELD_SIZEOF(struct sock_common,
					  skc_v6_rcv_saddr.s6_addr32[0]) != 4);

		off = si->off;
		off -= offsetof(struct __sk_buff, local_ip6[0]);

		*insn++ = BPF_LDX_MEM(BPF_FIELD_SIZEOF(struct sk_buff, sk),
				      si->dst_reg, si->src_reg,
				      offsetof(struct sk_buff, sk));
		*insn++ = BPF_LDX_MEM(BPF_W, si->dst_reg, si->dst_reg,
				      offsetof(struct sock_common,
					       skc_v6_rcv_saddr.s6_addr32[0]) +
				      off);
#else
		*insn++ = BPF_MOV32_IMM(si->dst_reg, 0);
#endif
		break;

	case offsetof(struct __sk_buff, remote_port):
		BUILD_BUG_ON(FIELD_SIZEOF(struct sock_common, skc_dport) != 2);

		*insn++ = BPF_LDX_MEM(BPF_FIELD_SIZEOF(struct sk_buff, sk),
				      si->dst_reg, si->src_reg,
				      offsetof(struct sk_buff, sk));
		*insn++ = BPF_LDX_MEM(BPF_H, si->dst_reg, si->dst_reg,
				      bpf_target_off(struct sock_common,
						     skc_dport,
						     2, target_size));
#ifndef __BIG_ENDIAN_BITFIELD
		*insn++ = BPF_ALU32_IMM(BPF_LSH, si->dst_reg, 16);
#endif
		break;

	case offsetof(struct __sk_buff, local_port):
		BUILD_BUG_ON(FIELD_SIZEOF(struct sock_common, skc_num) != 2);

		*insn++ = BPF_LDX_MEM(BPF_FIELD_SIZEOF(struct sk_buff, sk),
				      si->dst_reg, si->src_reg,
				      offsetof(struct sk_buff, sk));
		*insn++ = BPF_LDX_MEM(BPF_H, si->dst_reg, si->dst_reg,
				      bpf_target_off(struct sock_common,
						     skc_num, 2, target_size));
		break;
	}

	return insn - insn_buf;
}
	case offsetof(struct __sk_buff, local_port):
		BUILD_BUG_ON(FIELD_SIZEOF(struct sock_common, skc_num) != 2);

		*insn++ = BPF_LDX_MEM(BPF_FIELD_SIZEOF(struct sk_buff, sk),
				      si->dst_reg, si->src_reg,
				      offsetof(struct sk_buff, sk));
		*insn++ = BPF_LDX_MEM(BPF_H, si->dst_reg, si->dst_reg,
				      bpf_target_off(struct sock_common,
						     skc_num, 2, target_size));
		break;
	}

	return insn - insn_buf;
}

static u32 sock_filter_convert_ctx_access(enum bpf_access_type type,
					  const struct bpf_insn *si,
					  struct bpf_insn *insn_buf,
					  struct bpf_prog *prog, u32 *target_size)
{
	struct bpf_insn *insn = insn_buf;
	int off;

	switch (si->off) {
	case offsetof(struct bpf_sock, bound_dev_if):
		BUILD_BUG_ON(FIELD_SIZEOF(struct sock, sk_bound_dev_if) != 4);

		if (type == BPF_WRITE)
			*insn++ = BPF_STX_MEM(BPF_W, si->dst_reg, si->src_reg,
					offsetof(struct sock, sk_bound_dev_if));
		else
			*insn++ = BPF_LDX_MEM(BPF_W, si->dst_reg, si->src_reg,
				      offsetof(struct sock, sk_bound_dev_if));
		break;

	case offsetof(struct bpf_sock, mark):
		BUILD_BUG_ON(FIELD_SIZEOF(struct sock, sk_mark) != 4);

		if (type == BPF_WRITE)
			*insn++ = BPF_STX_MEM(BPF_W, si->dst_reg, si->src_reg,
					offsetof(struct sock, sk_mark));
		else
			*insn++ = BPF_LDX_MEM(BPF_W, si->dst_reg, si->src_reg,
				      offsetof(struct sock, sk_mark));
		break;

	case offsetof(struct bpf_sock, priority):
		BUILD_BUG_ON(FIELD_SIZEOF(struct sock, sk_priority) != 4);

		if (type == BPF_WRITE)
			*insn++ = BPF_STX_MEM(BPF_W, si->dst_reg, si->src_reg,
					offsetof(struct sock, sk_priority));
		else
			*insn++ = BPF_LDX_MEM(BPF_W, si->dst_reg, si->src_reg,
				      offsetof(struct sock, sk_priority));
		break;

	case offsetof(struct bpf_sock, family):
		BUILD_BUG_ON(FIELD_SIZEOF(struct sock, sk_family) != 2);

		*insn++ = BPF_LDX_MEM(BPF_H, si->dst_reg, si->src_reg,
				      offsetof(struct sock, sk_family));
		break;

	case offsetof(struct bpf_sock, type):
		*insn++ = BPF_LDX_MEM(BPF_W, si->dst_reg, si->src_reg,
				      offsetof(struct sock, __sk_flags_offset));
		*insn++ = BPF_ALU32_IMM(BPF_AND, si->dst_reg, SK_FL_TYPE_MASK);
		*insn++ = BPF_ALU32_IMM(BPF_RSH, si->dst_reg, SK_FL_TYPE_SHIFT);
		break;

	case offsetof(struct bpf_sock, protocol):
		*insn++ = BPF_LDX_MEM(BPF_W, si->dst_reg, si->src_reg,
				      offsetof(struct sock, __sk_flags_offset));
		*insn++ = BPF_ALU32_IMM(BPF_AND, si->dst_reg, SK_FL_PROTO_MASK);
		*insn++ = BPF_ALU32_IMM(BPF_RSH, si->dst_reg, SK_FL_PROTO_SHIFT);
		break;

	case offsetof(struct bpf_sock, src_ip4):
		*insn++ = BPF_LDX_MEM(
			BPF_SIZE(si->code), si->dst_reg, si->src_reg,
			bpf_target_off(struct sock_common, skc_rcv_saddr,
				       FIELD_SIZEOF(struct sock_common,
						    skc_rcv_saddr),
				       target_size));
		break;

	case bpf_ctx_range_till(struct bpf_sock, src_ip6[0], src_ip6[3]):
#if IS_ENABLED(CONFIG_IPV6)
		off = si->off;
		off -= offsetof(struct bpf_sock, src_ip6[0]);
		*insn++ = BPF_LDX_MEM(
			BPF_SIZE(si->code), si->dst_reg, si->src_reg,
			bpf_target_off(
				struct sock_common,
				skc_v6_rcv_saddr.s6_addr32[0],
				FIELD_SIZEOF(struct sock_common,
					     skc_v6_rcv_saddr.s6_addr32[0]),
				target_size) + off);
#else
		(void)off;
		*insn++ = BPF_MOV32_IMM(si->dst_reg, 0);
#endif
		break;

	case offsetof(struct bpf_sock, src_port):
		*insn++ = BPF_LDX_MEM(
			BPF_FIELD_SIZEOF(struct sock_common, skc_num),
			si->dst_reg, si->src_reg,
			bpf_target_off(struct sock_common, skc_num,
				       FIELD_SIZEOF(struct sock_common,
						    skc_num),
				       target_size));
		break;
	}

	return insn - insn_buf;
}

static u32 tc_cls_act_convert_ctx_access(enum bpf_access_type type,
					 const struct bpf_insn *si,
					 struct bpf_insn *insn_buf,
					 struct bpf_prog *prog, u32 *target_size)
{
	struct bpf_insn *insn = insn_buf;

	switch (si->off) {
	case offsetof(struct __sk_buff, ifindex):
		*insn++ = BPF_LDX_MEM(BPF_FIELD_SIZEOF(struct sk_buff, dev),
				      si->dst_reg, si->src_reg,
				      offsetof(struct sk_buff, dev));
		*insn++ = BPF_LDX_MEM(BPF_W, si->dst_reg, si->dst_reg,
				      bpf_target_off(struct net_device, ifindex, 4,
						     target_size));
		break;
	default:
		return bpf_convert_ctx_access(type, si, insn_buf, prog,
					      target_size);
	}

	return insn - insn_buf;
}

static u32 xdp_convert_ctx_access(enum bpf_access_type type,
				  const struct bpf_insn *si,
				  struct bpf_insn *insn_buf,
				  struct bpf_prog *prog, u32 *target_size)
{
	struct bpf_insn *insn = insn_buf;

	switch (si->off) {
	case offsetof(struct xdp_md, data):
		*insn++ = BPF_LDX_MEM(BPF_FIELD_SIZEOF(struct xdp_buff, data),
				      si->dst_reg, si->src_reg,
				      offsetof(struct xdp_buff, data));
		break;
	case offsetof(struct xdp_md, data_meta):
		*insn++ = BPF_LDX_MEM(BPF_FIELD_SIZEOF(struct xdp_buff, data_meta),
				      si->dst_reg, si->src_reg,
				      offsetof(struct xdp_buff, data_meta));
		break;
	case offsetof(struct xdp_md, data_end):
		*insn++ = BPF_LDX_MEM(BPF_FIELD_SIZEOF(struct xdp_buff, data_end),
				      si->dst_reg, si->src_reg,
				      offsetof(struct xdp_buff, data_end));
		break;
	case offsetof(struct xdp_md, ingress_ifindex):
		*insn++ = BPF_LDX_MEM(BPF_FIELD_SIZEOF(struct xdp_buff, rxq),
				      si->dst_reg, si->src_reg,
				      offsetof(struct xdp_buff, rxq));
		*insn++ = BPF_LDX_MEM(BPF_FIELD_SIZEOF(struct xdp_rxq_info, dev),
				      si->dst_reg, si->dst_reg,
				      offsetof(struct xdp_rxq_info, dev));
		*insn++ = BPF_LDX_MEM(BPF_W, si->dst_reg, si->dst_reg,
				      offsetof(struct net_device, ifindex));
		break;
	case offsetof(struct xdp_md, rx_queue_index):
		*insn++ = BPF_LDX_MEM(BPF_FIELD_SIZEOF(struct xdp_buff, rxq),
				      si->dst_reg, si->src_reg,
				      offsetof(struct xdp_buff, rxq));
		*insn++ = BPF_LDX_MEM(BPF_W, si->dst_reg, si->dst_reg,
				      offsetof(struct xdp_rxq_info,
					       queue_index));
		break;
	}

	return insn - insn_buf;
}

/* SOCK_ADDR_LOAD_NESTED_FIELD() loads Nested Field S.F.NF where S is type of
 * context Structure, F is Field in context structure that contains a pointer
 * to Nested Structure of type NS that has the field NF.
 *
 * SIZE encodes the load size (BPF_B, BPF_H, etc). It's up to caller to make
 * sure that SIZE is not greater than actual size of S.F.NF.
 *
 * If offset OFF is provided, the load happens from that offset relative to
 * offset of NF.
 */
#define SOCK_ADDR_LOAD_NESTED_FIELD_SIZE_OFF(S, NS, F, NF, SIZE, OFF)	       \
	do {								       \
		*insn++ = BPF_LDX_MEM(BPF_FIELD_SIZEOF(S, F), si->dst_reg,     \
				      si->src_reg, offsetof(S, F));	       \
		*insn++ = BPF_LDX_MEM(					       \
			SIZE, si->dst_reg, si->dst_reg,			       \
			bpf_target_off(NS, NF, FIELD_SIZEOF(NS, NF),	       \
				       target_size)			       \
				+ OFF);					       \
	} while (0)

#define SOCK_ADDR_LOAD_NESTED_FIELD(S, NS, F, NF)			       \
	SOCK_ADDR_LOAD_NESTED_FIELD_SIZE_OFF(S, NS, F, NF,		       \
					     BPF_FIELD_SIZEOF(NS, NF), 0)

/* SOCK_ADDR_STORE_NESTED_FIELD_OFF() has semantic similar to
 * SOCK_ADDR_LOAD_NESTED_FIELD_SIZE_OFF() but for store operation.
 *
 * It doesn't support SIZE argument though since narrow stores are not
 * supported for now.
 *
 * In addition it uses Temporary Field TF (member of struct S) as the 3rd
 * "register" since two registers available in convert_ctx_access are not
 * enough: we can't override neither SRC, since it contains value to store, nor
 * DST since it contains pointer to context that may be used by later
 * instructions. But we need a temporary place to save pointer to nested
 * structure whose field we want to store to.
 */
#define SOCK_ADDR_STORE_NESTED_FIELD_OFF(S, NS, F, NF, OFF, TF)		       \
	do {								       \
		int tmp_reg = BPF_REG_9;				       \
		if (si->src_reg == tmp_reg || si->dst_reg == tmp_reg)	       \
			--tmp_reg;					       \
		if (si->src_reg == tmp_reg || si->dst_reg == tmp_reg)	       \
			--tmp_reg;					       \
		*insn++ = BPF_STX_MEM(BPF_DW, si->dst_reg, tmp_reg,	       \
				      offsetof(S, TF));			       \
		*insn++ = BPF_LDX_MEM(BPF_FIELD_SIZEOF(S, F), tmp_reg,	       \
				      si->dst_reg, offsetof(S, F));	       \
		*insn++ = BPF_STX_MEM(					       \
			BPF_FIELD_SIZEOF(NS, NF), tmp_reg, si->src_reg,	       \
			bpf_target_off(NS, NF, FIELD_SIZEOF(NS, NF),	       \
				       target_size)			       \
				+ OFF);					       \
		*insn++ = BPF_LDX_MEM(BPF_DW, tmp_reg, si->dst_reg,	       \
				      offsetof(S, TF));			       \
	} while (0)

#define SOCK_ADDR_LOAD_OR_STORE_NESTED_FIELD_SIZE_OFF(S, NS, F, NF, SIZE, OFF, \
						      TF)		       \
	do {								       \
		if (type == BPF_WRITE) {				       \
			SOCK_ADDR_STORE_NESTED_FIELD_OFF(S, NS, F, NF, OFF,    \
							 TF);		       \
		} else {						       \
			SOCK_ADDR_LOAD_NESTED_FIELD_SIZE_OFF(		       \
				S, NS, F, NF, SIZE, OFF);  \
		}							       \
	} while (0)

#define SOCK_ADDR_LOAD_OR_STORE_NESTED_FIELD(S, NS, F, NF, TF)		       \
	SOCK_ADDR_LOAD_OR_STORE_NESTED_FIELD_SIZE_OFF(			       \
		S, NS, F, NF, BPF_FIELD_SIZEOF(NS, NF), 0, TF)

static u32 sock_addr_convert_ctx_access(enum bpf_access_type type,
					const struct bpf_insn *si,
					struct bpf_insn *insn_buf,
					struct bpf_prog *prog, u32 *target_size)
{
	struct bpf_insn *insn = insn_buf;
	int off;

	switch (si->off) {
	case offsetof(struct bpf_sock_addr, user_family):
		SOCK_ADDR_LOAD_NESTED_FIELD(struct bpf_sock_addr_kern,
					    struct sockaddr, uaddr, sa_family);
		break;

	case offsetof(struct bpf_sock_addr, user_ip4):
		SOCK_ADDR_LOAD_OR_STORE_NESTED_FIELD_SIZE_OFF(
			struct bpf_sock_addr_kern, struct sockaddr_in, uaddr,
			sin_addr, BPF_SIZE(si->code), 0, tmp_reg);
		break;

	case bpf_ctx_range_till(struct bpf_sock_addr, user_ip6[0], user_ip6[3]):
		off = si->off;
		off -= offsetof(struct bpf_sock_addr, user_ip6[0]);
		SOCK_ADDR_LOAD_OR_STORE_NESTED_FIELD_SIZE_OFF(
			struct bpf_sock_addr_kern, struct sockaddr_in6, uaddr,
			sin6_addr.s6_addr32[0], BPF_SIZE(si->code), off,
			tmp_reg);
		break;

	case offsetof(struct bpf_sock_addr, user_port):
		/* To get port we need to know sa_family first and then treat
		 * sockaddr as either sockaddr_in or sockaddr_in6.
		 * Though we can simplify since port field has same offset and
		 * size in both structures.
		 * Here we check this invariant and use just one of the
		 * structures if it's true.
		 */
		BUILD_BUG_ON(offsetof(struct sockaddr_in, sin_port) !=
			     offsetof(struct sockaddr_in6, sin6_port));
		BUILD_BUG_ON(FIELD_SIZEOF(struct sockaddr_in, sin_port) !=
			     FIELD_SIZEOF(struct sockaddr_in6, sin6_port));
		SOCK_ADDR_LOAD_OR_STORE_NESTED_FIELD(struct bpf_sock_addr_kern,
						     struct sockaddr_in6, uaddr,
						     sin6_port, tmp_reg);
		break;

	case offsetof(struct bpf_sock_addr, family):
		SOCK_ADDR_LOAD_NESTED_FIELD(struct bpf_sock_addr_kern,
					    struct sock, sk, sk_family);
		break;

	case offsetof(struct bpf_sock_addr, type):
		SOCK_ADDR_LOAD_NESTED_FIELD_SIZE_OFF(
			struct bpf_sock_addr_kern, struct sock, sk,
			__sk_flags_offset, BPF_W, 0);
		*insn++ = BPF_ALU32_IMM(BPF_AND, si->dst_reg, SK_FL_TYPE_MASK);
		*insn++ = BPF_ALU32_IMM(BPF_RSH, si->dst_reg, SK_FL_TYPE_SHIFT);
		break;

	case offsetof(struct bpf_sock_addr, protocol):
		SOCK_ADDR_LOAD_NESTED_FIELD_SIZE_OFF(
			struct bpf_sock_addr_kern, struct sock, sk,
			__sk_flags_offset, BPF_W, 0);
		*insn++ = BPF_ALU32_IMM(BPF_AND, si->dst_reg, SK_FL_PROTO_MASK);
		*insn++ = BPF_ALU32_IMM(BPF_RSH, si->dst_reg,
					SK_FL_PROTO_SHIFT);
		break;

	case offsetof(struct bpf_sock_addr, msg_src_ip4):
		/* Treat t_ctx as struct in_addr for msg_src_ip4. */
		SOCK_ADDR_LOAD_OR_STORE_NESTED_FIELD_SIZE_OFF(
			struct bpf_sock_addr_kern, struct in_addr, t_ctx,
			s_addr, BPF_SIZE(si->code), 0, tmp_reg);
		break;

	case bpf_ctx_range_till(struct bpf_sock_addr, msg_src_ip6[0],
				msg_src_ip6[3]):
		off = si->off;
		off -= offsetof(struct bpf_sock_addr, msg_src_ip6[0]);
		/* Treat t_ctx as struct in6_addr for msg_src_ip6. */
		SOCK_ADDR_LOAD_OR_STORE_NESTED_FIELD_SIZE_OFF(
			struct bpf_sock_addr_kern, struct in6_addr, t_ctx,
			s6_addr32[0], BPF_SIZE(si->code), off, tmp_reg);
		break;
	}

	return insn - insn_buf;
}

static u32 sock_ops_convert_ctx_access(enum bpf_access_type type,
				       const struct bpf_insn *si,
				       struct bpf_insn *insn_buf,
				       struct bpf_prog *prog,
				       u32 *target_size)
{
	struct bpf_insn *insn = insn_buf;
	int off;

	switch (si->off) {
	case offsetof(struct bpf_sock_ops, op) ...
	     offsetof(struct bpf_sock_ops, replylong[3]):
		BUILD_BUG_ON(FIELD_SIZEOF(struct bpf_sock_ops, op) !=
			     FIELD_SIZEOF(struct bpf_sock_ops_kern, op));
		BUILD_BUG_ON(FIELD_SIZEOF(struct bpf_sock_ops, reply) !=
			     FIELD_SIZEOF(struct bpf_sock_ops_kern, reply));
		BUILD_BUG_ON(FIELD_SIZEOF(struct bpf_sock_ops, replylong) !=
			     FIELD_SIZEOF(struct bpf_sock_ops_kern, replylong));
		off = si->off;
		off -= offsetof(struct bpf_sock_ops, op);
		off += offsetof(struct bpf_sock_ops_kern, op);
		if (type == BPF_WRITE)
			*insn++ = BPF_STX_MEM(BPF_W, si->dst_reg, si->src_reg,
					      off);
		else
			*insn++ = BPF_LDX_MEM(BPF_W, si->dst_reg, si->src_reg,
					      off);
		break;

	case offsetof(struct bpf_sock_ops, family):
		BUILD_BUG_ON(FIELD_SIZEOF(struct sock_common, skc_family) != 2);

		*insn++ = BPF_LDX_MEM(BPF_FIELD_SIZEOF(
					      struct bpf_sock_ops_kern, sk),
				      si->dst_reg, si->src_reg,
				      offsetof(struct bpf_sock_ops_kern, sk));
		*insn++ = BPF_LDX_MEM(BPF_H, si->dst_reg, si->dst_reg,
				      offsetof(struct sock_common, skc_family));
		break;

	case offsetof(struct bpf_sock_ops, remote_ip4):
		BUILD_BUG_ON(FIELD_SIZEOF(struct sock_common, skc_daddr) != 4);

		*insn++ = BPF_LDX_MEM(BPF_FIELD_SIZEOF(
						struct bpf_sock_ops_kern, sk),
				      si->dst_reg, si->src_reg,
				      offsetof(struct bpf_sock_ops_kern, sk));
		*insn++ = BPF_LDX_MEM(BPF_W, si->dst_reg, si->dst_reg,
				      offsetof(struct sock_common, skc_daddr));
		break;

	case offsetof(struct bpf_sock_ops, local_ip4):
		BUILD_BUG_ON(FIELD_SIZEOF(struct sock_common,
					  skc_rcv_saddr) != 4);

		*insn++ = BPF_LDX_MEM(BPF_FIELD_SIZEOF(
					      struct bpf_sock_ops_kern, sk),
				      si->dst_reg, si->src_reg,
				      offsetof(struct bpf_sock_ops_kern, sk));
		*insn++ = BPF_LDX_MEM(BPF_W, si->dst_reg, si->dst_reg,
				      offsetof(struct sock_common,
					       skc_rcv_saddr));
		break;

	case offsetof(struct bpf_sock_ops, remote_ip6[0]) ...
	     offsetof(struct bpf_sock_ops, remote_ip6[3]):
#if IS_ENABLED(CONFIG_IPV6)
		BUILD_BUG_ON(FIELD_SIZEOF(struct sock_common,
					  skc_v6_daddr.s6_addr32[0]) != 4);

		off = si->off;
		off -= offsetof(struct bpf_sock_ops, remote_ip6[0]);
		*insn++ = BPF_LDX_MEM(BPF_FIELD_SIZEOF(
						struct bpf_sock_ops_kern, sk),
				      si->dst_reg, si->src_reg,
				      offsetof(struct bpf_sock_ops_kern, sk));
		*insn++ = BPF_LDX_MEM(BPF_W, si->dst_reg, si->dst_reg,
				      offsetof(struct sock_common,
					       skc_v6_daddr.s6_addr32[0]) +
				      off);
#else
		*insn++ = BPF_MOV32_IMM(si->dst_reg, 0);
#endif
		break;

	case offsetof(struct bpf_sock_ops, local_ip6[0]) ...
	     offsetof(struct bpf_sock_ops, local_ip6[3]):
#if IS_ENABLED(CONFIG_IPV6)
		BUILD_BUG_ON(FIELD_SIZEOF(struct sock_common,
					  skc_v6_rcv_saddr.s6_addr32[0]) != 4);

		off = si->off;
		off -= offsetof(struct bpf_sock_ops, local_ip6[0]);
		*insn++ = BPF_LDX_MEM(BPF_FIELD_SIZEOF(
						struct bpf_sock_ops_kern, sk),
				      si->dst_reg, si->src_reg,
				      offsetof(struct bpf_sock_ops_kern, sk));
		*insn++ = BPF_LDX_MEM(BPF_W, si->dst_reg, si->dst_reg,
				      offsetof(struct sock_common,
					       skc_v6_rcv_saddr.s6_addr32[0]) +
				      off);
#else
		*insn++ = BPF_MOV32_IMM(si->dst_reg, 0);
#endif
		break;

	case offsetof(struct bpf_sock_ops, remote_port):
		BUILD_BUG_ON(FIELD_SIZEOF(struct sock_common, skc_dport) != 2);

		*insn++ = BPF_LDX_MEM(BPF_FIELD_SIZEOF(
						struct bpf_sock_ops_kern, sk),
				      si->dst_reg, si->src_reg,
				      offsetof(struct bpf_sock_ops_kern, sk));
		*insn++ = BPF_LDX_MEM(BPF_H, si->dst_reg, si->dst_reg,
				      offsetof(struct sock_common, skc_dport));
#ifndef __BIG_ENDIAN_BITFIELD
		*insn++ = BPF_ALU32_IMM(BPF_LSH, si->dst_reg, 16);
#endif
		break;

	case offsetof(struct bpf_sock_ops, local_port):
		BUILD_BUG_ON(FIELD_SIZEOF(struct sock_common, skc_num) != 2);

		*insn++ = BPF_LDX_MEM(BPF_FIELD_SIZEOF(
						struct bpf_sock_ops_kern, sk),
				      si->dst_reg, si->src_reg,
				      offsetof(struct bpf_sock_ops_kern, sk));
		*insn++ = BPF_LDX_MEM(BPF_H, si->dst_reg, si->dst_reg,
				      offsetof(struct sock_common, skc_num));
		break;

	case offsetof(struct bpf_sock_ops, is_fullsock):
		*insn++ = BPF_LDX_MEM(BPF_FIELD_SIZEOF(
						struct bpf_sock_ops_kern,
						is_fullsock),
				      si->dst_reg, si->src_reg,
				      offsetof(struct bpf_sock_ops_kern,
					       is_fullsock));
		break;

	case offsetof(struct bpf_sock_ops, state):
		BUILD_BUG_ON(FIELD_SIZEOF(struct sock_common, skc_state) != 1);

		*insn++ = BPF_LDX_MEM(BPF_FIELD_SIZEOF(
						struct bpf_sock_ops_kern, sk),
				      si->dst_reg, si->src_reg,
				      offsetof(struct bpf_sock_ops_kern, sk));
		*insn++ = BPF_LDX_MEM(BPF_B, si->dst_reg, si->dst_reg,
				      offsetof(struct sock_common, skc_state));
		break;

	case offsetof(struct bpf_sock_ops, rtt_min):
		BUILD_BUG_ON(FIELD_SIZEOF(struct tcp_sock, rtt_min) !=
			     sizeof(struct minmax));
		BUILD_BUG_ON(sizeof(struct minmax) <
			     sizeof(struct minmax_sample));

		*insn++ = BPF_LDX_MEM(BPF_FIELD_SIZEOF(
						struct bpf_sock_ops_kern, sk),
				      si->dst_reg, si->src_reg,
				      offsetof(struct bpf_sock_ops_kern, sk));
		*insn++ = BPF_LDX_MEM(BPF_W, si->dst_reg, si->dst_reg,
				      offsetof(struct tcp_sock, rtt_min) +
				      FIELD_SIZEOF(struct minmax_sample, t));
		break;

/* Helper macro for adding read access to tcp_sock or sock fields. */
#define SOCK_OPS_GET_FIELD(BPF_FIELD, OBJ_FIELD, OBJ)			      \
	do {								      \
		BUILD_BUG_ON(FIELD_SIZEOF(OBJ, OBJ_FIELD) >		      \
			     FIELD_SIZEOF(struct bpf_sock_ops, BPF_FIELD));   \
		*insn++ = BPF_LDX_MEM(BPF_FIELD_SIZEOF(			      \
						struct bpf_sock_ops_kern,     \
						is_fullsock),		      \
				      si->dst_reg, si->src_reg,		      \
				      offsetof(struct bpf_sock_ops_kern,      \
					       is_fullsock));		      \
		*insn++ = BPF_JMP_IMM(BPF_JEQ, si->dst_reg, 0, 2);	      \
		*insn++ = BPF_LDX_MEM(BPF_FIELD_SIZEOF(			      \
						struct bpf_sock_ops_kern, sk),\
				      si->dst_reg, si->src_reg,		      \
				      offsetof(struct bpf_sock_ops_kern, sk));\
		*insn++ = BPF_LDX_MEM(BPF_FIELD_SIZEOF(OBJ,		      \
						       OBJ_FIELD),	      \
				      si->dst_reg, si->dst_reg,		      \
				      offsetof(OBJ, OBJ_FIELD));	      \
	} while (0)

/* Helper macro for adding write access to tcp_sock or sock fields.
 * The macro is called with two registers, dst_reg which contains a pointer
 * to ctx (context) and src_reg which contains the value that should be
 * stored. However, we need an additional register since we cannot overwrite
 * dst_reg because it may be used later in the program.
 * Instead we "borrow" one of the other register. We first save its value
 * into a new (temp) field in bpf_sock_ops_kern, use it, and then restore
 * it at the end of the macro.
 */
#define SOCK_OPS_SET_FIELD(BPF_FIELD, OBJ_FIELD, OBJ)			      \
	do {								      \
		int reg = BPF_REG_9;					      \
		BUILD_BUG_ON(FIELD_SIZEOF(OBJ, OBJ_FIELD) >		      \
			     FIELD_SIZEOF(struct bpf_sock_ops, BPF_FIELD));   \
		if (si->dst_reg == reg || si->src_reg == reg)		      \
			reg--;						      \
		if (si->dst_reg == reg || si->src_reg == reg)		      \
			reg--;						      \
		*insn++ = BPF_STX_MEM(BPF_DW, si->dst_reg, reg,		      \
				      offsetof(struct bpf_sock_ops_kern,      \
					       temp));			      \
		*insn++ = BPF_LDX_MEM(BPF_FIELD_SIZEOF(			      \
						struct bpf_sock_ops_kern,     \
						is_fullsock),		      \
				      reg, si->dst_reg,			      \
				      offsetof(struct bpf_sock_ops_kern,      \
					       is_fullsock));		      \
		*insn++ = BPF_JMP_IMM(BPF_JEQ, reg, 0, 2);		      \
		*insn++ = BPF_LDX_MEM(BPF_FIELD_SIZEOF(			      \
						struct bpf_sock_ops_kern, sk),\
				      reg, si->dst_reg,			      \
				      offsetof(struct bpf_sock_ops_kern, sk));\
		*insn++ = BPF_STX_MEM(BPF_FIELD_SIZEOF(OBJ, OBJ_FIELD),	      \
				      reg, si->src_reg,			      \
				      offsetof(OBJ, OBJ_FIELD));	      \
		*insn++ = BPF_LDX_MEM(BPF_DW, reg, si->dst_reg,		      \
				      offsetof(struct bpf_sock_ops_kern,      \
					       temp));			      \
	} while (0)

#define SOCK_OPS_GET_OR_SET_FIELD(BPF_FIELD, OBJ_FIELD, OBJ, TYPE)	      \
	do {								      \
		if (TYPE == BPF_WRITE)					      \
			SOCK_OPS_SET_FIELD(BPF_FIELD, OBJ_FIELD, OBJ);	      \
		else							      \
			SOCK_OPS_GET_FIELD(BPF_FIELD, OBJ_FIELD, OBJ);	      \
	} while (0)

	case offsetof(struct bpf_sock_ops, snd_cwnd):
		SOCK_OPS_GET_FIELD(snd_cwnd, snd_cwnd, struct tcp_sock);
		break;

	case offsetof(struct bpf_sock_ops, srtt_us):
		SOCK_OPS_GET_FIELD(srtt_us, srtt_us, struct tcp_sock);
		break;

	case offsetof(struct bpf_sock_ops, bpf_sock_ops_cb_flags):
		SOCK_OPS_GET_FIELD(bpf_sock_ops_cb_flags, bpf_sock_ops_cb_flags,
				   struct tcp_sock);
		break;

	case offsetof(struct bpf_sock_ops, snd_ssthresh):
		SOCK_OPS_GET_FIELD(snd_ssthresh, snd_ssthresh, struct tcp_sock);
		break;

	case offsetof(struct bpf_sock_ops, rcv_nxt):
		SOCK_OPS_GET_FIELD(rcv_nxt, rcv_nxt, struct tcp_sock);
		break;

	case offsetof(struct bpf_sock_ops, snd_nxt):
		SOCK_OPS_GET_FIELD(snd_nxt, snd_nxt, struct tcp_sock);
		break;

	case offsetof(struct bpf_sock_ops, snd_una):
		SOCK_OPS_GET_FIELD(snd_una, snd_una, struct tcp_sock);
		break;

	case offsetof(struct bpf_sock_ops, mss_cache):
		SOCK_OPS_GET_FIELD(mss_cache, mss_cache, struct tcp_sock);
		break;

	case offsetof(struct bpf_sock_ops, ecn_flags):
		SOCK_OPS_GET_FIELD(ecn_flags, ecn_flags, struct tcp_sock);
		break;

	case offsetof(struct bpf_sock_ops, rate_delivered):
		SOCK_OPS_GET_FIELD(rate_delivered, rate_delivered,
				   struct tcp_sock);
		break;

	case offsetof(struct bpf_sock_ops, rate_interval_us):
		SOCK_OPS_GET_FIELD(rate_interval_us, rate_interval_us,
				   struct tcp_sock);
		break;

	case offsetof(struct bpf_sock_ops, packets_out):
		SOCK_OPS_GET_FIELD(packets_out, packets_out, struct tcp_sock);
		break;

	case offsetof(struct bpf_sock_ops, retrans_out):
		SOCK_OPS_GET_FIELD(retrans_out, retrans_out, struct tcp_sock);
		break;

	case offsetof(struct bpf_sock_ops, total_retrans):
		SOCK_OPS_GET_FIELD(total_retrans, total_retrans,
				   struct tcp_sock);
		break;

	case offsetof(struct bpf_sock_ops, segs_in):
		SOCK_OPS_GET_FIELD(segs_in, segs_in, struct tcp_sock);
		break;

	case offsetof(struct bpf_sock_ops, data_segs_in):
		SOCK_OPS_GET_FIELD(data_segs_in, data_segs_in, struct tcp_sock);
		break;

	case offsetof(struct bpf_sock_ops, segs_out):
		SOCK_OPS_GET_FIELD(segs_out, segs_out, struct tcp_sock);
		break;

	case offsetof(struct bpf_sock_ops, data_segs_out):
		SOCK_OPS_GET_FIELD(data_segs_out, data_segs_out,
				   struct tcp_sock);
		break;

	case offsetof(struct bpf_sock_ops, lost_out):
		SOCK_OPS_GET_FIELD(lost_out, lost_out, struct tcp_sock);
		break;

	case offsetof(struct bpf_sock_ops, sacked_out):
		SOCK_OPS_GET_FIELD(sacked_out, sacked_out, struct tcp_sock);
		break;

	case offsetof(struct bpf_sock_ops, sk_txhash):
		SOCK_OPS_GET_OR_SET_FIELD(sk_txhash, sk_txhash,
					  struct sock, type);
		break;

	case offsetof(struct bpf_sock_ops, bytes_received):
		SOCK_OPS_GET_FIELD(bytes_received, bytes_received,
				   struct tcp_sock);
		break;

	case offsetof(struct bpf_sock_ops, bytes_acked):
		SOCK_OPS_GET_FIELD(bytes_acked, bytes_acked, struct tcp_sock);
		break;

	}
	return insn - insn_buf;
}

static u32 sk_skb_convert_ctx_access(enum bpf_access_type type,
				     const struct bpf_insn *si,
				     struct bpf_insn *insn_buf,
				     struct bpf_prog *prog, u32 *target_size)
{
	struct bpf_insn *insn = insn_buf;
	int off;

	switch (si->off) {
	case offsetof(struct __sk_buff, data_end):
		off  = si->off;
		off -= offsetof(struct __sk_buff, data_end);
		off += offsetof(struct sk_buff, cb);
		off += offsetof(struct tcp_skb_cb, bpf.data_end);
		*insn++ = BPF_LDX_MEM(BPF_SIZEOF(void *), si->dst_reg,
				      si->src_reg, off);
		break;
	default:
		return bpf_convert_ctx_access(type, si, insn_buf, prog,
					      target_size);
	}

	return insn - insn_buf;
}

static u32 sk_msg_convert_ctx_access(enum bpf_access_type type,
				     const struct bpf_insn *si,
				     struct bpf_insn *insn_buf,
				     struct bpf_prog *prog, u32 *target_size)
{
	struct bpf_insn *insn = insn_buf;
#if IS_ENABLED(CONFIG_IPV6)
	int off;
#endif

	switch (si->off) {
	case offsetof(struct sk_msg_md, data):
		*insn++ = BPF_LDX_MEM(BPF_FIELD_SIZEOF(struct sk_msg_buff, data),
				      si->dst_reg, si->src_reg,
				      offsetof(struct sk_msg_buff, data));
		break;
	case offsetof(struct sk_msg_md, data_end):
		*insn++ = BPF_LDX_MEM(BPF_FIELD_SIZEOF(struct sk_msg_buff, data_end),
				      si->dst_reg, si->src_reg,
				      offsetof(struct sk_msg_buff, data_end));
		break;
	case offsetof(struct sk_msg_md, family):
		BUILD_BUG_ON(FIELD_SIZEOF(struct sock_common, skc_family) != 2);

		*insn++ = BPF_LDX_MEM(BPF_FIELD_SIZEOF(
					      struct sk_msg_buff, sk),
				      si->dst_reg, si->src_reg,
				      offsetof(struct sk_msg_buff, sk));
		*insn++ = BPF_LDX_MEM(BPF_H, si->dst_reg, si->dst_reg,
				      offsetof(struct sock_common, skc_family));
		break;

	case offsetof(struct sk_msg_md, remote_ip4):
		BUILD_BUG_ON(FIELD_SIZEOF(struct sock_common, skc_daddr) != 4);

		*insn++ = BPF_LDX_MEM(BPF_FIELD_SIZEOF(
						struct sk_msg_buff, sk),
				      si->dst_reg, si->src_reg,
				      offsetof(struct sk_msg_buff, sk));
		*insn++ = BPF_LDX_MEM(BPF_W, si->dst_reg, si->dst_reg,
				      offsetof(struct sock_common, skc_daddr));
		break;

	case offsetof(struct sk_msg_md, local_ip4):
		BUILD_BUG_ON(FIELD_SIZEOF(struct sock_common,
					  skc_rcv_saddr) != 4);

		*insn++ = BPF_LDX_MEM(BPF_FIELD_SIZEOF(
					      struct sk_msg_buff, sk),
				      si->dst_reg, si->src_reg,
				      offsetof(struct sk_msg_buff, sk));
		*insn++ = BPF_LDX_MEM(BPF_W, si->dst_reg, si->dst_reg,
				      offsetof(struct sock_common,
					       skc_rcv_saddr));
		break;

	case offsetof(struct sk_msg_md, remote_ip6[0]) ...
	     offsetof(struct sk_msg_md, remote_ip6[3]):
#if IS_ENABLED(CONFIG_IPV6)
		BUILD_BUG_ON(FIELD_SIZEOF(struct sock_common,
					  skc_v6_daddr.s6_addr32[0]) != 4);

		off = si->off;
		off -= offsetof(struct sk_msg_md, remote_ip6[0]);
		*insn++ = BPF_LDX_MEM(BPF_FIELD_SIZEOF(
						struct sk_msg_buff, sk),
				      si->dst_reg, si->src_reg,
				      offsetof(struct sk_msg_buff, sk));
		*insn++ = BPF_LDX_MEM(BPF_W, si->dst_reg, si->dst_reg,
				      offsetof(struct sock_common,
					       skc_v6_daddr.s6_addr32[0]) +
				      off);
#else
		*insn++ = BPF_MOV32_IMM(si->dst_reg, 0);
#endif
		break;

	case offsetof(struct sk_msg_md, local_ip6[0]) ...
	     offsetof(struct sk_msg_md, local_ip6[3]):
#if IS_ENABLED(CONFIG_IPV6)
		BUILD_BUG_ON(FIELD_SIZEOF(struct sock_common,
					  skc_v6_rcv_saddr.s6_addr32[0]) != 4);

		off = si->off;
		off -= offsetof(struct sk_msg_md, local_ip6[0]);
		*insn++ = BPF_LDX_MEM(BPF_FIELD_SIZEOF(
						struct sk_msg_buff, sk),
				      si->dst_reg, si->src_reg,
				      offsetof(struct sk_msg_buff, sk));
		*insn++ = BPF_LDX_MEM(BPF_W, si->dst_reg, si->dst_reg,
				      offsetof(struct sock_common,
					       skc_v6_rcv_saddr.s6_addr32[0]) +
				      off);
#else
		*insn++ = BPF_MOV32_IMM(si->dst_reg, 0);
#endif
		break;

	case offsetof(struct sk_msg_md, remote_port):
		BUILD_BUG_ON(FIELD_SIZEOF(struct sock_common, skc_dport) != 2);

		*insn++ = BPF_LDX_MEM(BPF_FIELD_SIZEOF(
						struct sk_msg_buff, sk),
				      si->dst_reg, si->src_reg,
				      offsetof(struct sk_msg_buff, sk));
		*insn++ = BPF_LDX_MEM(BPF_H, si->dst_reg, si->dst_reg,
				      offsetof(struct sock_common, skc_dport));
#ifndef __BIG_ENDIAN_BITFIELD
		*insn++ = BPF_ALU32_IMM(BPF_LSH, si->dst_reg, 16);
#endif
		break;

	case offsetof(struct sk_msg_md, local_port):
		BUILD_BUG_ON(FIELD_SIZEOF(struct sock_common, skc_num) != 2);

		*insn++ = BPF_LDX_MEM(BPF_FIELD_SIZEOF(
						struct sk_msg_buff, sk),
				      si->dst_reg, si->src_reg,
				      offsetof(struct sk_msg_buff, sk));
		*insn++ = BPF_LDX_MEM(BPF_H, si->dst_reg, si->dst_reg,
				      offsetof(struct sock_common, skc_num));
		break;
	}

	return insn - insn_buf;
}

const struct bpf_verifier_ops sk_filter_verifier_ops = {
	.get_func_proto		= sk_filter_func_proto,
	.is_valid_access	= sk_filter_is_valid_access,
	.convert_ctx_access	= bpf_convert_ctx_access,
	.gen_ld_abs		= bpf_gen_ld_abs,
};

const struct bpf_prog_ops sk_filter_prog_ops = {
	.test_run		= bpf_prog_test_run_skb,
};

const struct bpf_verifier_ops tc_cls_act_verifier_ops = {
	.get_func_proto		= tc_cls_act_func_proto,
	.is_valid_access	= tc_cls_act_is_valid_access,
	.convert_ctx_access	= tc_cls_act_convert_ctx_access,
	.gen_prologue		= tc_cls_act_prologue,
	.gen_ld_abs		= bpf_gen_ld_abs,
};

const struct bpf_prog_ops tc_cls_act_prog_ops = {
	.test_run		= bpf_prog_test_run_skb,
};

const struct bpf_verifier_ops xdp_verifier_ops = {
	.get_func_proto		= xdp_func_proto,
	.is_valid_access	= xdp_is_valid_access,
	.convert_ctx_access	= xdp_convert_ctx_access,
};

const struct bpf_prog_ops xdp_prog_ops = {
	.test_run		= bpf_prog_test_run_xdp,
};

const struct bpf_verifier_ops cg_skb_verifier_ops = {
	.get_func_proto		= cg_skb_func_proto,
	.is_valid_access	= sk_filter_is_valid_access,
	.convert_ctx_access	= bpf_convert_ctx_access,
};

const struct bpf_prog_ops cg_skb_prog_ops = {
	.test_run		= bpf_prog_test_run_skb,
};

const struct bpf_verifier_ops lwt_in_verifier_ops = {
	.get_func_proto		= lwt_in_func_proto,
	.is_valid_access	= lwt_is_valid_access,
	.convert_ctx_access	= bpf_convert_ctx_access,
};

const struct bpf_prog_ops lwt_in_prog_ops = {
	.test_run		= bpf_prog_test_run_skb,
};

const struct bpf_verifier_ops lwt_out_verifier_ops = {
	.get_func_proto		= lwt_out_func_proto,
	.is_valid_access	= lwt_is_valid_access,
	.convert_ctx_access	= bpf_convert_ctx_access,
};

const struct bpf_prog_ops lwt_out_prog_ops = {
	.test_run		= bpf_prog_test_run_skb,
};

const struct bpf_verifier_ops lwt_xmit_verifier_ops = {
	.get_func_proto		= lwt_xmit_func_proto,
	.is_valid_access	= lwt_is_valid_access,
	.convert_ctx_access	= bpf_convert_ctx_access,
	.gen_prologue		= tc_cls_act_prologue,
};

const struct bpf_prog_ops lwt_xmit_prog_ops = {
	.test_run		= bpf_prog_test_run_skb,
};

const struct bpf_verifier_ops lwt_seg6local_verifier_ops = {
	.get_func_proto		= lwt_seg6local_func_proto,
	.is_valid_access	= lwt_is_valid_access,
	.convert_ctx_access	= bpf_convert_ctx_access,
};

const struct bpf_prog_ops lwt_seg6local_prog_ops = {
	.test_run		= bpf_prog_test_run_skb,
};

const struct bpf_verifier_ops cg_sock_verifier_ops = {
	.get_func_proto		= sock_filter_func_proto,
	.is_valid_access	= sock_filter_is_valid_access,
	.convert_ctx_access	= sock_filter_convert_ctx_access,
};

const struct bpf_prog_ops cg_sock_prog_ops = {
};

const struct bpf_verifier_ops cg_sock_addr_verifier_ops = {
	.get_func_proto		= sock_addr_func_proto,
	.is_valid_access	= sock_addr_is_valid_access,
	.convert_ctx_access	= sock_addr_convert_ctx_access,
};

const struct bpf_prog_ops cg_sock_addr_prog_ops = {
};

const struct bpf_verifier_ops sock_ops_verifier_ops = {
	.get_func_proto		= sock_ops_func_proto,
	.is_valid_access	= sock_ops_is_valid_access,
	.convert_ctx_access	= sock_ops_convert_ctx_access,
};

const struct bpf_prog_ops sock_ops_prog_ops = {
};

const struct bpf_verifier_ops sk_skb_verifier_ops = {
	.get_func_proto		= sk_skb_func_proto,
	.is_valid_access	= sk_skb_is_valid_access,
	.convert_ctx_access	= sk_skb_convert_ctx_access,
	.gen_prologue		= sk_skb_prologue,
};

const struct bpf_prog_ops sk_skb_prog_ops = {
};

const struct bpf_verifier_ops sk_msg_verifier_ops = {
	.get_func_proto		= sk_msg_func_proto,
	.is_valid_access	= sk_msg_is_valid_access,
	.convert_ctx_access	= sk_msg_convert_ctx_access,
};

const struct bpf_prog_ops sk_msg_prog_ops = {
};

int sk_detach_filter(struct sock *sk)
{
	int ret = -ENOENT;
	struct sk_filter *filter;

	if (sock_flag(sk, SOCK_FILTER_LOCKED))
		return -EPERM;

	filter = rcu_dereference_protected(sk->sk_filter,
					   lockdep_sock_is_held(sk));
	if (filter) {
		RCU_INIT_POINTER(sk->sk_filter, NULL);
		sk_filter_uncharge(sk, filter);
		ret = 0;
	}

	return ret;
}
EXPORT_SYMBOL_GPL(sk_detach_filter);

int sk_get_filter(struct sock *sk, struct sock_filter __user *ubuf,
		  unsigned int len)
{
	struct sock_fprog_kern *fprog;
	struct sk_filter *filter;
	int ret = 0;

	lock_sock(sk);
	filter = rcu_dereference_protected(sk->sk_filter,
					   lockdep_sock_is_held(sk));
	if (!filter)
		goto out;

	/* We're copying the filter that has been originally attached,
	 * so no conversion/decode needed anymore. eBPF programs that
	 * have no original program cannot be dumped through this.
	 */
	ret = -EACCES;
	fprog = filter->prog->orig_prog;
	if (!fprog)
		goto out;

	ret = fprog->len;
	if (!len)
		/* User space only enquires number of filter blocks. */
		goto out;

	ret = -EINVAL;
	if (len < fprog->len)
		goto out;

	ret = -EFAULT;
	if (copy_to_user(ubuf, fprog->filter, bpf_classic_proglen(fprog)))
		goto out;

	/* Instead of bytes, the API requests to return the number
	 * of filter blocks.
	 */
	ret = fprog->len;
out:
	release_sock(sk);
	return ret;
}

#ifdef CONFIG_INET
struct sk_reuseport_kern {
	struct sk_buff *skb;
	struct sock *sk;
	struct sock *selected_sk;
	void *data_end;
	u32 hash;
	u32 reuseport_id;
	bool bind_inany;
};

static void bpf_init_reuseport_kern(struct sk_reuseport_kern *reuse_kern,
				    struct sock_reuseport *reuse,
				    struct sock *sk, struct sk_buff *skb,
				    u32 hash)
{
	reuse_kern->skb = skb;
	reuse_kern->sk = sk;
	reuse_kern->selected_sk = NULL;
	reuse_kern->data_end = skb->data + skb_headlen(skb);
	reuse_kern->hash = hash;
	reuse_kern->reuseport_id = reuse->reuseport_id;
	reuse_kern->bind_inany = reuse->bind_inany;
}

struct sock *bpf_run_sk_reuseport(struct sock_reuseport *reuse, struct sock *sk,
				  struct bpf_prog *prog, struct sk_buff *skb,
				  u32 hash)
{
	struct sk_reuseport_kern reuse_kern;
	enum sk_action action;

	bpf_init_reuseport_kern(&reuse_kern, reuse, sk, skb, hash);
	action = BPF_PROG_RUN(prog, &reuse_kern);

	if (action == SK_PASS)
		return reuse_kern.selected_sk;
	else
		return ERR_PTR(-ECONNREFUSED);
}

BPF_CALL_4(sk_select_reuseport, struct sk_reuseport_kern *, reuse_kern,
	   struct bpf_map *, map, void *, key, u32, flags)
{
	struct sock_reuseport *reuse;
	struct sock *selected_sk;

	selected_sk = map->ops->map_lookup_elem(map, key);
	if (!selected_sk)
		return -ENOENT;

	reuse = rcu_dereference(selected_sk->sk_reuseport_cb);
	if (!reuse)
		/* selected_sk is unhashed (e.g. by close()) after the
		 * above map_lookup_elem().  Treat selected_sk has already
		 * been removed from the map.
		 */
		return -ENOENT;

	if (unlikely(reuse->reuseport_id != reuse_kern->reuseport_id)) {
		struct sock *sk;

		if (unlikely(!reuse_kern->reuseport_id))
			/* There is a small race between adding the
			 * sk to the map and setting the
			 * reuse_kern->reuseport_id.
			 * Treat it as the sk has not been added to
			 * the bpf map yet.
			 */
			return -ENOENT;

		sk = reuse_kern->sk;
		if (sk->sk_protocol != selected_sk->sk_protocol)
			return -EPROTOTYPE;
		else if (sk->sk_family != selected_sk->sk_family)
			return -EAFNOSUPPORT;

		/* Catch all. Likely bound to a different sockaddr. */
		return -EBADFD;
	}

	reuse_kern->selected_sk = selected_sk;

	return 0;
}

static const struct bpf_func_proto sk_select_reuseport_proto = {
	.func           = sk_select_reuseport,
	.gpl_only       = false,
	.ret_type       = RET_INTEGER,
	.arg1_type	= ARG_PTR_TO_CTX,
	.arg2_type      = ARG_CONST_MAP_PTR,
	.arg3_type      = ARG_PTR_TO_MAP_KEY,
	.arg4_type	= ARG_ANYTHING,
};

BPF_CALL_4(sk_reuseport_load_bytes,
	   const struct sk_reuseport_kern *, reuse_kern, u32, offset,
	   void *, to, u32, len)
{
	return ____bpf_skb_load_bytes(reuse_kern->skb, offset, to, len);
}

static const struct bpf_func_proto sk_reuseport_load_bytes_proto = {
	.func		= sk_reuseport_load_bytes,
	.gpl_only	= false,
	.ret_type	= RET_INTEGER,
	.arg1_type	= ARG_PTR_TO_CTX,
	.arg2_type	= ARG_ANYTHING,
	.arg3_type	= ARG_PTR_TO_UNINIT_MEM,
	.arg4_type	= ARG_CONST_SIZE,
};

BPF_CALL_5(sk_reuseport_load_bytes_relative,
	   const struct sk_reuseport_kern *, reuse_kern, u32, offset,
	   void *, to, u32, len, u32, start_header)
{
	return ____bpf_skb_load_bytes_relative(reuse_kern->skb, offset, to,
					       len, start_header);
}

static const struct bpf_func_proto sk_reuseport_load_bytes_relative_proto = {
	.func		= sk_reuseport_load_bytes_relative,
	.gpl_only	= false,
	.ret_type	= RET_INTEGER,
	.arg1_type	= ARG_PTR_TO_CTX,
	.arg2_type	= ARG_ANYTHING,
	.arg3_type	= ARG_PTR_TO_UNINIT_MEM,
	.arg4_type	= ARG_CONST_SIZE,
	.arg5_type	= ARG_ANYTHING,
};

static const struct bpf_func_proto *
sk_reuseport_func_proto(enum bpf_func_id func_id,
			const struct bpf_prog *prog)
{
	switch (func_id) {
	case BPF_FUNC_sk_select_reuseport:
		return &sk_select_reuseport_proto;
	case BPF_FUNC_skb_load_bytes:
		return &sk_reuseport_load_bytes_proto;
	case BPF_FUNC_skb_load_bytes_relative:
		return &sk_reuseport_load_bytes_relative_proto;
	default:
		return bpf_base_func_proto(func_id);
	}
}

static bool
sk_reuseport_is_valid_access(int off, int size,
			     enum bpf_access_type type,
			     const struct bpf_prog *prog,
			     struct bpf_insn_access_aux *info)
{
	const u32 size_default = sizeof(__u32);

	if (off < 0 || off >= sizeof(struct sk_reuseport_md) ||
	    off % size || type != BPF_READ)
		return false;

	switch (off) {
	case offsetof(struct sk_reuseport_md, data):
		info->reg_type = PTR_TO_PACKET;
		return size == sizeof(__u64);

	case offsetof(struct sk_reuseport_md, data_end):
		info->reg_type = PTR_TO_PACKET_END;
		return size == sizeof(__u64);

	case offsetof(struct sk_reuseport_md, hash):
		return size == size_default;

	/* Fields that allow narrowing */
	case offsetof(struct sk_reuseport_md, eth_protocol):
		if (size < FIELD_SIZEOF(struct sk_buff, protocol))
			return false;
		/* fall through */
	case offsetof(struct sk_reuseport_md, ip_protocol):
	case offsetof(struct sk_reuseport_md, bind_inany):
	case offsetof(struct sk_reuseport_md, len):
		bpf_ctx_record_field_size(info, size_default);
		return bpf_ctx_narrow_access_ok(off, size, size_default);

	default:
		return false;
	}
}

#define SK_REUSEPORT_LOAD_FIELD(F) ({					\
	*insn++ = BPF_LDX_MEM(BPF_FIELD_SIZEOF(struct sk_reuseport_kern, F), \
			      si->dst_reg, si->src_reg,			\
			      bpf_target_off(struct sk_reuseport_kern, F, \
					     FIELD_SIZEOF(struct sk_reuseport_kern, F), \
					     target_size));		\
	})

#define SK_REUSEPORT_LOAD_SKB_FIELD(SKB_FIELD)				\
	SOCK_ADDR_LOAD_NESTED_FIELD(struct sk_reuseport_kern,		\
				    struct sk_buff,			\
				    skb,				\
				    SKB_FIELD)

#define SK_REUSEPORT_LOAD_SK_FIELD_SIZE_OFF(SK_FIELD, BPF_SIZE, EXTRA_OFF) \
	SOCK_ADDR_LOAD_NESTED_FIELD_SIZE_OFF(struct sk_reuseport_kern,	\
					     struct sock,		\
					     sk,			\
					     SK_FIELD, BPF_SIZE, EXTRA_OFF)

static u32 sk_reuseport_convert_ctx_access(enum bpf_access_type type,
					   const struct bpf_insn *si,
					   struct bpf_insn *insn_buf,
					   struct bpf_prog *prog,
					   u32 *target_size)
{
	struct bpf_insn *insn = insn_buf;

	switch (si->off) {
	case offsetof(struct sk_reuseport_md, data):
		SK_REUSEPORT_LOAD_SKB_FIELD(data);
		break;

	case offsetof(struct sk_reuseport_md, len):
		SK_REUSEPORT_LOAD_SKB_FIELD(len);
		break;

	case offsetof(struct sk_reuseport_md, eth_protocol):
		SK_REUSEPORT_LOAD_SKB_FIELD(protocol);
		break;

	case offsetof(struct sk_reuseport_md, ip_protocol):
		BUILD_BUG_ON(HWEIGHT32(SK_FL_PROTO_MASK) != BITS_PER_BYTE);
		SK_REUSEPORT_LOAD_SK_FIELD_SIZE_OFF(__sk_flags_offset,
						    BPF_W, 0);
		*insn++ = BPF_ALU32_IMM(BPF_AND, si->dst_reg, SK_FL_PROTO_MASK);
		*insn++ = BPF_ALU32_IMM(BPF_RSH, si->dst_reg,
					SK_FL_PROTO_SHIFT);
		/* SK_FL_PROTO_MASK and SK_FL_PROTO_SHIFT are endian
		 * aware.  No further narrowing or masking is needed.
		 */
		*target_size = 1;
		break;

	case offsetof(struct sk_reuseport_md, data_end):
		SK_REUSEPORT_LOAD_FIELD(data_end);
		break;

	case offsetof(struct sk_reuseport_md, hash):
		SK_REUSEPORT_LOAD_FIELD(hash);
		break;

	case offsetof(struct sk_reuseport_md, bind_inany):
		SK_REUSEPORT_LOAD_FIELD(bind_inany);
		break;
	}

	return insn - insn_buf;
}

const struct bpf_verifier_ops sk_reuseport_verifier_ops = {
	.get_func_proto		= sk_reuseport_func_proto,
	.is_valid_access	= sk_reuseport_is_valid_access,
	.convert_ctx_access	= sk_reuseport_convert_ctx_access,
};

const struct bpf_prog_ops sk_reuseport_prog_ops = {
};
#endif /* CONFIG_INET */
const struct bpf_prog_ops sk_msg_prog_ops = {
};

int sk_detach_filter(struct sock *sk)
{
	int ret = -ENOENT;
	struct sk_filter *filter;

	if (sock_flag(sk, SOCK_FILTER_LOCKED))
		return -EPERM;

	filter = rcu_dereference_protected(sk->sk_filter,
					   lockdep_sock_is_held(sk));
	if (filter) {
		RCU_INIT_POINTER(sk->sk_filter, NULL);
		sk_filter_uncharge(sk, filter);
		ret = 0;
	}

	return ret;
}