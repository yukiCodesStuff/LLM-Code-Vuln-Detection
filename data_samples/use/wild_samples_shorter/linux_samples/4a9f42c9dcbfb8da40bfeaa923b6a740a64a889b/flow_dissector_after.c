#include <net/flow_dissector.h>
#include <scsi/fc/fc_fcoe.h>
#include <uapi/linux/batadv_packet.h>
#include <linux/bpf.h>

static DEFINE_MUTEX(flow_dissector_mutex);

static void dissector_set_key(struct flow_dissector *flow_dissector,
			      enum flow_dissector_key_id key_id)
{
}
EXPORT_SYMBOL(skb_flow_dissector_init);

int skb_flow_dissector_bpf_prog_attach(const union bpf_attr *attr,
				       struct bpf_prog *prog)
{
	struct bpf_prog *attached;
	struct net *net;

	net = current->nsproxy->net_ns;
	mutex_lock(&flow_dissector_mutex);
	attached = rcu_dereference_protected(net->flow_dissector_prog,
					     lockdep_is_held(&flow_dissector_mutex));
	if (attached) {
		/* Only one BPF program can be attached at a time */
		mutex_unlock(&flow_dissector_mutex);
		return -EEXIST;
	}
	rcu_assign_pointer(net->flow_dissector_prog, prog);
	mutex_unlock(&flow_dissector_mutex);
	return 0;
}

int skb_flow_dissector_bpf_prog_detach(const union bpf_attr *attr)
{
	struct bpf_prog *attached;
	struct net *net;

	net = current->nsproxy->net_ns;
	mutex_lock(&flow_dissector_mutex);
	attached = rcu_dereference_protected(net->flow_dissector_prog,
					     lockdep_is_held(&flow_dissector_mutex));
	if (!attached) {
		mutex_unlock(&flow_dissector_mutex);
		return -ENOENT;
	}
	bpf_prog_put(attached);
	RCU_INIT_POINTER(net->flow_dissector_prog, NULL);
	mutex_unlock(&flow_dissector_mutex);
	return 0;
}
/**
 * skb_flow_get_be16 - extract be16 entity
 * @skb: sk_buff to extract from
 * @poff: offset to extract at
	return (*num_hdrs <= MAX_FLOW_DISSECT_HDRS);
}

static void __skb_flow_bpf_to_target(const struct bpf_flow_keys *flow_keys,
				     struct flow_dissector *flow_dissector,
				     void *target_container)
{
	struct flow_dissector_key_control *key_control;
	struct flow_dissector_key_basic *key_basic;
	struct flow_dissector_key_addrs *key_addrs;
	struct flow_dissector_key_ports *key_ports;

	key_control = skb_flow_dissector_target(flow_dissector,
						FLOW_DISSECTOR_KEY_CONTROL,
						target_container);
	key_control->thoff = flow_keys->thoff;
	if (flow_keys->is_frag)
		key_control->flags |= FLOW_DIS_IS_FRAGMENT;
	if (flow_keys->is_first_frag)
		key_control->flags |= FLOW_DIS_FIRST_FRAG;
	if (flow_keys->is_encap)
		key_control->flags |= FLOW_DIS_ENCAPSULATION;

	key_basic = skb_flow_dissector_target(flow_dissector,
					      FLOW_DISSECTOR_KEY_BASIC,
					      target_container);
	key_basic->n_proto = flow_keys->n_proto;
	key_basic->ip_proto = flow_keys->ip_proto;

	if (flow_keys->addr_proto == ETH_P_IP &&
	    dissector_uses_key(flow_dissector, FLOW_DISSECTOR_KEY_IPV4_ADDRS)) {
		key_addrs = skb_flow_dissector_target(flow_dissector,
						      FLOW_DISSECTOR_KEY_IPV4_ADDRS,
						      target_container);
		key_addrs->v4addrs.src = flow_keys->ipv4_src;
		key_addrs->v4addrs.dst = flow_keys->ipv4_dst;
		key_control->addr_type = FLOW_DISSECTOR_KEY_IPV4_ADDRS;
	} else if (flow_keys->addr_proto == ETH_P_IPV6 &&
		   dissector_uses_key(flow_dissector,
				      FLOW_DISSECTOR_KEY_IPV6_ADDRS)) {
		key_addrs = skb_flow_dissector_target(flow_dissector,
						      FLOW_DISSECTOR_KEY_IPV6_ADDRS,
						      target_container);
		memcpy(&key_addrs->v6addrs, &flow_keys->ipv6_src,
		       sizeof(key_addrs->v6addrs));
		key_control->addr_type = FLOW_DISSECTOR_KEY_IPV6_ADDRS;
	}

	if (dissector_uses_key(flow_dissector, FLOW_DISSECTOR_KEY_PORTS)) {
		key_ports = skb_flow_dissector_target(flow_dissector,
						      FLOW_DISSECTOR_KEY_PORTS,
						      target_container);
		key_ports->src = flow_keys->sport;
		key_ports->dst = flow_keys->dport;
	}
}

/**
 * __skb_flow_dissect - extract the flow_keys struct and return it
 * @skb: sk_buff to extract the flow from, can be NULL if the rest are specified
 * @flow_dissector: list of keys to dissect
	struct flow_dissector_key_vlan *key_vlan;
	enum flow_dissect_ret fdret;
	enum flow_dissector_key_id dissector_vlan = FLOW_DISSECTOR_KEY_MAX;
	struct bpf_prog *attached;
	int num_hdrs = 0;
	u8 ip_proto = 0;
	bool ret;

					      FLOW_DISSECTOR_KEY_BASIC,
					      target_container);

	rcu_read_lock();
	attached = skb ? rcu_dereference(dev_net(skb->dev)->flow_dissector_prog)
		       : NULL;
	if (attached) {
		/* Note that even though the const qualifier is discarded
		 * throughout the execution of the BPF program, all changes(the
		 * control block) are reverted after the BPF program returns.
		 * Therefore, __skb_flow_dissect does not alter the skb.
		 */
		struct bpf_flow_keys flow_keys = {};
		struct bpf_skb_data_end cb_saved;
		struct bpf_skb_data_end *cb;
		u32 result;

		cb = (struct bpf_skb_data_end *)skb->cb;

		/* Save Control Block */
		memcpy(&cb_saved, cb, sizeof(cb_saved));
		memset(cb, 0, sizeof(cb_saved));

		/* Pass parameters to the BPF program */
		cb->qdisc_cb.flow_keys = &flow_keys;
		flow_keys.nhoff = nhoff;

		bpf_compute_data_pointers((struct sk_buff *)skb);
		result = BPF_PROG_RUN(attached, skb);

		/* Restore state */
		memcpy(cb, &cb_saved, sizeof(cb_saved));

		__skb_flow_bpf_to_target(&flow_keys, flow_dissector,
					 target_container);
		key_control->thoff = min_t(u16, key_control->thoff, skb->len);
		rcu_read_unlock();
		return result == BPF_OK;
	}
	rcu_read_unlock();

	if (dissector_uses_key(flow_dissector,
			       FLOW_DISSECTOR_KEY_ETH_ADDRS)) {
		struct ethhdr *eth = eth_hdr(skb);
		struct flow_dissector_key_eth_addrs *key_eth_addrs;