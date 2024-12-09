#include <linux/kernel.h>
#include <linux/skbuff.h>
#include <linux/export.h>
#include <linux/ip.h>
#include <linux/ipv6.h>
#include <linux/if_vlan.h>
#include <net/dsa.h>
#include <net/dst_metadata.h>
#include <net/ip.h>
#include <net/ipv6.h>
#include <net/gre.h>
#include <net/pptp.h>
#include <net/tipc.h>
#include <linux/igmp.h>
#include <linux/icmp.h>
#include <linux/sctp.h>
#include <linux/dccp.h>
#include <linux/if_tunnel.h>
#include <linux/if_pppox.h>
#include <linux/ppp_defs.h>
#include <linux/stddef.h>
#include <linux/if_ether.h>
#include <linux/mpls.h>
#include <linux/tcp.h>
#include <net/flow_dissector.h>
#include <scsi/fc/fc_fcoe.h>
#include <uapi/linux/batadv_packet.h>
#include <linux/bpf.h>

static DEFINE_MUTEX(flow_dissector_mutex);

static void dissector_set_key(struct flow_dissector *flow_dissector,
			      enum flow_dissector_key_id key_id)
{
	flow_dissector->used_keys |= (1 << key_id);
}
	for (i = 0; i < key_count; i++, key++) {
		/* User should make sure that every key target offset is withing
		 * boundaries of unsigned short.
		 */
		BUG_ON(key->offset > USHRT_MAX);
		BUG_ON(dissector_uses_key(flow_dissector,
					  key->key_id));

		dissector_set_key(flow_dissector, key->key_id);
		flow_dissector->offset[key->key_id] = key->offset;
	}

	/* Ensure that the dissector always includes control and basic key.
	 * That way we are able to avoid handling lack of these in fast path.
	 */
	BUG_ON(!dissector_uses_key(flow_dissector,
				   FLOW_DISSECTOR_KEY_CONTROL));
	BUG_ON(!dissector_uses_key(flow_dissector,
				   FLOW_DISSECTOR_KEY_BASIC));
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
{
	++*num_hdrs;

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
{
	struct flow_dissector_key_control *key_control;
	struct flow_dissector_key_basic *key_basic;
	struct flow_dissector_key_addrs *key_addrs;
	struct flow_dissector_key_ports *key_ports;
	struct flow_dissector_key_icmp *key_icmp;
	struct flow_dissector_key_tags *key_tags;
	struct flow_dissector_key_vlan *key_vlan;
	enum flow_dissect_ret fdret;
	enum flow_dissector_key_id dissector_vlan = FLOW_DISSECTOR_KEY_MAX;
	struct bpf_prog *attached = NULL;
	int num_hdrs = 0;
	u8 ip_proto = 0;
	bool ret;

	if (!data) {
		data = skb->data;
		proto = skb_vlan_tag_present(skb) ?
			 skb->vlan_proto : skb->protocol;
		nhoff = skb_network_offset(skb);
		hlen = skb_headlen(skb);
#if IS_ENABLED(CONFIG_NET_DSA)
		if (unlikely(skb->dev && netdev_uses_dsa(skb->dev))) {
			const struct dsa_device_ops *ops;
			int offset;

			ops = skb->dev->dsa_ptr->tag_ops;
			if (ops->flow_dissect &&
			    !ops->flow_dissect(skb, &proto, &offset)) {
				hlen -= offset;
				nhoff += offset;
			}
		}
#endif
	}
			    !ops->flow_dissect(skb, &proto, &offset)) {
				hlen -= offset;
				nhoff += offset;
			}
		}
#endif
	}

	/* It is ensured by skb_flow_dissector_init() that control key will
	 * be always present.
	 */
	key_control = skb_flow_dissector_target(flow_dissector,
						FLOW_DISSECTOR_KEY_CONTROL,
						target_container);

	/* It is ensured by skb_flow_dissector_init() that basic key will
	 * be always present.
	 */
	key_basic = skb_flow_dissector_target(flow_dissector,
					      FLOW_DISSECTOR_KEY_BASIC,
					      target_container);

	rcu_read_lock();
	if (skb) {
		if (skb->dev)
			attached = rcu_dereference(dev_net(skb->dev)->flow_dissector_prog);
		else if (skb->sk)
			attached = rcu_dereference(sock_net(skb->sk)->flow_dissector_prog);
		else
			WARN_ON_ONCE(1);
	}