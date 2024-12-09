#include <net/flow_dissector.h>
#include <scsi/fc/fc_fcoe.h>
#include <uapi/linux/batadv_packet.h>

static void dissector_set_key(struct flow_dissector *flow_dissector,
			      enum flow_dissector_key_id key_id)
{
}
EXPORT_SYMBOL(skb_flow_dissector_init);

/**
 * skb_flow_get_be16 - extract be16 entity
 * @skb: sk_buff to extract from
 * @poff: offset to extract at
	return (*num_hdrs <= MAX_FLOW_DISSECT_HDRS);
}

/**
 * __skb_flow_dissect - extract the flow_keys struct and return it
 * @skb: sk_buff to extract the flow from, can be NULL if the rest are specified
 * @flow_dissector: list of keys to dissect
	struct flow_dissector_key_vlan *key_vlan;
	enum flow_dissect_ret fdret;
	enum flow_dissector_key_id dissector_vlan = FLOW_DISSECTOR_KEY_MAX;
	int num_hdrs = 0;
	u8 ip_proto = 0;
	bool ret;

					      FLOW_DISSECTOR_KEY_BASIC,
					      target_container);

	if (dissector_uses_key(flow_dissector,
			       FLOW_DISSECTOR_KEY_ETH_ADDRS)) {
		struct ethhdr *eth = eth_hdr(skb);
		struct flow_dissector_key_eth_addrs *key_eth_addrs;