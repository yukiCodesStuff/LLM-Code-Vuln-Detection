struct napi_gro_cb {
	/* Virtual address of skb_shinfo(skb)->frags[0].page + offset. */
	void	*frag0;

	/* Length of frag0. */
	unsigned int frag0_len;

	/* This indicates where we are processing relative to skb->data. */
	int	data_offset;

	/* This is non-zero if the packet cannot be merged with the new skb. */
	u16	flush;

	/* Save the IP ID here and check when we get to the transport layer */
	u16	flush_id;

	/* Number of segments aggregated. */
	u16	count;

	/* Start offset for remote checksum offload */
	u16	gro_remcsum_start;

	/* jiffies when first packet was created/queued */
	unsigned long age;

	/* Used in ipv6_gro_receive() and foo-over-udp */
	u16	proto;

	/* This is non-zero if the packet may be of the same flow. */
	u8	same_flow:1;

	/* Used in tunnel GRO receive */
	u8	encap_mark:1;

	/* GRO checksum is valid */
	u8	csum_valid:1;

	/* Number of checksums via CHECKSUM_UNNECESSARY */
	u8	csum_cnt:3;

	/* Free the skb? */
	u8	free:2;
#define NAPI_GRO_FREE		  1
#define NAPI_GRO_FREE_STOLEN_HEAD 2

	/* Used in foo-over-udp, set in udp[46]_gro_receive */
	u8	is_ipv6:1;

	/* Used in GRE, set in fou/gue_gro_receive */
	u8	is_fou:1;

	/* Used to determine if flush_id can be ignored */
	u8	is_atomic:1;

	/* Number of gro_receive callbacks this packet already went through */
	u8 recursion_counter:4;

	/* 1 bit hole */

	/* used to support CHECKSUM_COMPLETE for tunneling protocols */
	__wsum	csum;

	/* used in skb_gro_receive() slow path */
	struct sk_buff *last;
};

#define NAPI_GRO_CB(skb) ((struct napi_gro_cb *)(skb)->cb)

#define GRO_RECURSION_LIMIT 15
static inline int gro_recursion_inc_test(struct sk_buff *skb)
{
	return ++NAPI_GRO_CB(skb)->recursion_counter == GRO_RECURSION_LIMIT;
}
struct napi_gro_cb {
	/* Virtual address of skb_shinfo(skb)->frags[0].page + offset. */
	void	*frag0;

	/* Length of frag0. */
	unsigned int frag0_len;

	/* This indicates where we are processing relative to skb->data. */
	int	data_offset;

	/* This is non-zero if the packet cannot be merged with the new skb. */
	u16	flush;

	/* Save the IP ID here and check when we get to the transport layer */
	u16	flush_id;

	/* Number of segments aggregated. */
	u16	count;

	/* Start offset for remote checksum offload */
	u16	gro_remcsum_start;

	/* jiffies when first packet was created/queued */
	unsigned long age;

	/* Used in ipv6_gro_receive() and foo-over-udp */
	u16	proto;

	/* This is non-zero if the packet may be of the same flow. */
	u8	same_flow:1;

	/* Used in tunnel GRO receive */
	u8	encap_mark:1;

	/* GRO checksum is valid */
	u8	csum_valid:1;

	/* Number of checksums via CHECKSUM_UNNECESSARY */
	u8	csum_cnt:3;

	/* Free the skb? */
	u8	free:2;
#define NAPI_GRO_FREE		  1
#define NAPI_GRO_FREE_STOLEN_HEAD 2

	/* Used in foo-over-udp, set in udp[46]_gro_receive */
	u8	is_ipv6:1;

	/* Used in GRE, set in fou/gue_gro_receive */
	u8	is_fou:1;

	/* Used to determine if flush_id can be ignored */
	u8	is_atomic:1;

	/* Number of gro_receive callbacks this packet already went through */
	u8 recursion_counter:4;

	/* 1 bit hole */

	/* used to support CHECKSUM_COMPLETE for tunneling protocols */
	__wsum	csum;

	/* used in skb_gro_receive() slow path */
	struct sk_buff *last;
};

#define NAPI_GRO_CB(skb) ((struct napi_gro_cb *)(skb)->cb)

#define GRO_RECURSION_LIMIT 15
static inline int gro_recursion_inc_test(struct sk_buff *skb)
{
	return ++NAPI_GRO_CB(skb)->recursion_counter == GRO_RECURSION_LIMIT;
}