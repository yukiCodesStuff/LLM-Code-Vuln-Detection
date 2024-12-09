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

/**
 * skb_flow_get_be16 - extract be16 entity
 * @skb: sk_buff to extract from
 * @poff: offset to extract at
 * @data: raw buffer pointer to the packet
 * @hlen: packet header length
 *
 * The function will try to retrieve a be32 entity at
 * offset poff
 */
static __be16 skb_flow_get_be16(const struct sk_buff *skb, int poff,
				void *data, int hlen)
{
	__be16 *u, _u;

	u = __skb_header_pointer(skb, poff, sizeof(_u), data, hlen, &_u);
	if (u)
		return *u;

	return 0;
}
{
	++*num_hdrs;

	return (*num_hdrs <= MAX_FLOW_DISSECT_HDRS);
}

/**
 * __skb_flow_dissect - extract the flow_keys struct and return it
 * @skb: sk_buff to extract the flow from, can be NULL if the rest are specified
 * @flow_dissector: list of keys to dissect
 * @target_container: target structure to put dissected values into
 * @data: raw buffer pointer to the packet, if NULL use skb->data
 * @proto: protocol for which to get the flow, if @data is NULL use skb->protocol
 * @nhoff: network header offset, if @data is NULL use skb_network_offset(skb)
 * @hlen: packet header length, if @data is NULL use skb_headlen(skb)
 *
 * The function will try to retrieve individual keys into target specified
 * by flow_dissector from either the skbuff or a raw buffer specified by the
 * rest parameters.
 *
 * Caller must take care of zeroing target container memory.
 */
bool __skb_flow_dissect(const struct sk_buff *skb,
			struct flow_dissector *flow_dissector,
			void *target_container,
			void *data, __be16 proto, int nhoff, int hlen,
			unsigned int flags)
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

	if (dissector_uses_key(flow_dissector,
			       FLOW_DISSECTOR_KEY_ETH_ADDRS)) {
		struct ethhdr *eth = eth_hdr(skb);
		struct flow_dissector_key_eth_addrs *key_eth_addrs;

		key_eth_addrs = skb_flow_dissector_target(flow_dissector,
							  FLOW_DISSECTOR_KEY_ETH_ADDRS,
							  target_container);
		memcpy(key_eth_addrs, &eth->h_dest, sizeof(*key_eth_addrs));
	}