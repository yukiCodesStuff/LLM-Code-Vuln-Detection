	/* Used to determine if flush_id can be ignored */
	u8	is_atomic:1;

	/* Number of gro_receive callbacks this packet already went through */
	u8 recursion_counter:4;

	/* 1 bit hole */

	/* used to support CHECKSUM_COMPLETE for tunneling protocols */
	__wsum	csum;


#define NAPI_GRO_CB(skb) ((struct napi_gro_cb *)(skb)->cb)

#define GRO_RECURSION_LIMIT 15
static inline int gro_recursion_inc_test(struct sk_buff *skb)
{
	return ++NAPI_GRO_CB(skb)->recursion_counter == GRO_RECURSION_LIMIT;
}

typedef struct sk_buff **(*gro_receive_t)(struct sk_buff **, struct sk_buff *);
static inline struct sk_buff **call_gro_receive(gro_receive_t cb,
						struct sk_buff **head,
						struct sk_buff *skb)
{
	if (unlikely(gro_recursion_inc_test(skb))) {
		NAPI_GRO_CB(skb)->flush |= 1;
		return NULL;
	}

	return cb(head, skb);
}

typedef struct sk_buff **(*gro_receive_sk_t)(struct sock *, struct sk_buff **,
					     struct sk_buff *);
static inline struct sk_buff **call_gro_receive_sk(gro_receive_sk_t cb,
						   struct sock *sk,
						   struct sk_buff **head,
						   struct sk_buff *skb)
{
	if (unlikely(gro_recursion_inc_test(skb))) {
		NAPI_GRO_CB(skb)->flush |= 1;
		return NULL;
	}

	return cb(sk, head, skb);
}

struct packet_type {
	__be16			type;	/* This is really htons(ether_type). */
	struct net_device	*dev;	/* NULL is wildcarded here	     */
	int			(*func) (struct sk_buff *,