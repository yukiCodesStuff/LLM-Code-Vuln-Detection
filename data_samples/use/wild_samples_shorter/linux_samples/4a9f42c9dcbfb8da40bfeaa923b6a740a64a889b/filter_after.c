	}
}

static const struct bpf_func_proto *
flow_dissector_func_proto(enum bpf_func_id func_id, const struct bpf_prog *prog)
{
	switch (func_id) {
	case BPF_FUNC_skb_load_bytes:
		return &bpf_skb_load_bytes_proto;
	default:
		return bpf_base_func_proto(func_id);
	}
}

static const struct bpf_func_proto *
lwt_out_func_proto(enum bpf_func_id func_id, const struct bpf_prog *prog)
{
	switch (func_id) {
		if (size != size_default)
			return false;
		break;
	case bpf_ctx_range(struct __sk_buff, flow_keys):
		if (size != sizeof(struct bpf_flow_keys *))
			return false;
		break;
	default:
		/* Only narrow read access allowed for now. */
		if (type == BPF_WRITE) {
			if (size != size_default)
	case bpf_ctx_range(struct __sk_buff, data):
	case bpf_ctx_range(struct __sk_buff, data_meta):
	case bpf_ctx_range(struct __sk_buff, data_end):
	case bpf_ctx_range(struct __sk_buff, flow_keys):
	case bpf_ctx_range_till(struct __sk_buff, family, local_port):
		return false;
	}

	case bpf_ctx_range(struct __sk_buff, tc_classid):
	case bpf_ctx_range_till(struct __sk_buff, family, local_port):
	case bpf_ctx_range(struct __sk_buff, data_meta):
	case bpf_ctx_range(struct __sk_buff, flow_keys):
		return false;
	}

	if (type == BPF_WRITE) {
	case bpf_ctx_range(struct __sk_buff, data_end):
		info->reg_type = PTR_TO_PACKET_END;
		break;
	case bpf_ctx_range(struct __sk_buff, flow_keys):
	case bpf_ctx_range_till(struct __sk_buff, family, local_port):
		return false;
	}

	switch (off) {
	case bpf_ctx_range(struct __sk_buff, tc_classid):
	case bpf_ctx_range(struct __sk_buff, data_meta):
	case bpf_ctx_range(struct __sk_buff, flow_keys):
		return false;
	}

	if (type == BPF_WRITE) {
	return true;
}

static bool flow_dissector_is_valid_access(int off, int size,
					   enum bpf_access_type type,
					   const struct bpf_prog *prog,
					   struct bpf_insn_access_aux *info)
{
	if (type == BPF_WRITE) {
		switch (off) {
		case bpf_ctx_range_till(struct __sk_buff, cb[0], cb[4]):
			break;
		default:
			return false;
		}
	}

	switch (off) {
	case bpf_ctx_range(struct __sk_buff, data):
		info->reg_type = PTR_TO_PACKET;
		break;
	case bpf_ctx_range(struct __sk_buff, data_end):
		info->reg_type = PTR_TO_PACKET_END;
		break;
	case bpf_ctx_range(struct __sk_buff, flow_keys):
		info->reg_type = PTR_TO_FLOW_KEYS;
		break;
	case bpf_ctx_range(struct __sk_buff, tc_classid):
	case bpf_ctx_range(struct __sk_buff, data_meta):
	case bpf_ctx_range_till(struct __sk_buff, family, local_port):
		return false;
	}

	return bpf_skb_is_valid_access(off, size, type, prog, info);
}

static u32 bpf_convert_ctx_access(enum bpf_access_type type,
				  const struct bpf_insn *si,
				  struct bpf_insn *insn_buf,
				  struct bpf_prog *prog, u32 *target_size)
				      bpf_target_off(struct sock_common,
						     skc_num, 2, target_size));
		break;

	case offsetof(struct __sk_buff, flow_keys):
		off  = si->off;
		off -= offsetof(struct __sk_buff, flow_keys);
		off += offsetof(struct sk_buff, cb);
		off += offsetof(struct qdisc_skb_cb, flow_keys);
		*insn++ = BPF_LDX_MEM(BPF_SIZEOF(void *), si->dst_reg,
				      si->src_reg, off);
		break;
	}

	return insn - insn_buf;
}
const struct bpf_prog_ops sk_msg_prog_ops = {
};

const struct bpf_verifier_ops flow_dissector_verifier_ops = {
	.get_func_proto		= flow_dissector_func_proto,
	.is_valid_access	= flow_dissector_is_valid_access,
	.convert_ctx_access	= bpf_convert_ctx_access,
};

const struct bpf_prog_ops flow_dissector_prog_ops = {
};

int sk_detach_filter(struct sock *sk)
{
	int ret = -ENOENT;
	struct sk_filter *filter;