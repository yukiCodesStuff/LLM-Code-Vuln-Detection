		    !(iph->frag_off & htons(IP_DF))) {
			flush_id ^= NAPI_GRO_CB(p)->count;
			flush_id = flush_id ? 0xFFFF : 0;
		}

		/* If the previous IP ID value was based on an atomic
		 * datagram we can overwrite the value and ignore it.
		 */
		if (NAPI_GRO_CB(skb)->is_atomic)
			NAPI_GRO_CB(p)->flush_id = flush_id;
		else
			NAPI_GRO_CB(p)->flush_id |= flush_id;
	}

	NAPI_GRO_CB(skb)->is_atomic = !!(iph->frag_off & htons(IP_DF));
	NAPI_GRO_CB(skb)->flush |= flush;
	skb_set_network_header(skb, off);
	/* The above will be needed by the transport layer if there is one
	 * immediately following this IP hdr.
	 */

	/* Note : No need to call skb_gro_postpull_rcsum() here,
	 * as we already checked checksum over ipv4 header was 0
	 */
	skb_gro_pull(skb, sizeof(*iph));
	skb_set_transport_header(skb, skb_gro_offset(skb));

	pp = ops->callbacks.gro_receive(head, skb);

out_unlock:
	rcu_read_unlock();

out:
	NAPI_GRO_CB(skb)->flush |= flush;

	return pp;
}
EXPORT_SYMBOL(inet_gro_receive);

static struct sk_buff **ipip_gro_receive(struct sk_buff **head,
					 struct sk_buff *skb)
{
	if (NAPI_GRO_CB(skb)->encap_mark) {
		NAPI_GRO_CB(skb)->flush = 1;
		return NULL;
	}