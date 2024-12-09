	/* Used to determine if flush_id can be ignored */
	u8	is_atomic:1;

	/* 5 bit hole */

	/* used to support CHECKSUM_COMPLETE for tunneling protocols */
	__wsum	csum;


#define NAPI_GRO_CB(skb) ((struct napi_gro_cb *)(skb)->cb)

struct packet_type {
	__be16			type;	/* This is really htons(ether_type). */
	struct net_device	*dev;	/* NULL is wildcarded here	     */
	int			(*func) (struct sk_buff *,