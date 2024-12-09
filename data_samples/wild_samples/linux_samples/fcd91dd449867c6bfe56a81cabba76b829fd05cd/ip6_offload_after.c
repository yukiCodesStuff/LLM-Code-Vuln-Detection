			   nlen - offsetof(struct ipv6hdr, nexthdr))) {
			NAPI_GRO_CB(p)->same_flow = 0;
			continue;
		}
		/* flush if Traffic Class fields are different */
		NAPI_GRO_CB(p)->flush |= !!(first_word & htonl(0x0FF00000));
		NAPI_GRO_CB(p)->flush |= flush;

		/* If the previous IP ID value was based on an atomic
		 * datagram we can overwrite the value and ignore it.
		 */
		if (NAPI_GRO_CB(skb)->is_atomic)
			NAPI_GRO_CB(p)->flush_id = 0;
	}

	NAPI_GRO_CB(skb)->is_atomic = true;
	NAPI_GRO_CB(skb)->flush |= flush;

	skb_gro_postpull_rcsum(skb, iph, nlen);

	pp = call_gro_receive(ops->callbacks.gro_receive, head, skb);

out_unlock:
	rcu_read_unlock();

out:
	NAPI_GRO_CB(skb)->flush |= flush;

	return pp;
}

static struct sk_buff **sit_ip6ip6_gro_receive(struct sk_buff **head,
					       struct sk_buff *skb)
{
	/* Common GRO receive for SIT and IP6IP6 */

	if (NAPI_GRO_CB(skb)->encap_mark) {
		NAPI_GRO_CB(skb)->flush = 1;
		return NULL;
	}