enum sdp_header_types {
	SDP_HDR_UNSPEC,
	SDP_HDR_VERSION,
	SDP_HDR_OWNER_IP4,
	SDP_HDR_CONNECTION_IP4,
	SDP_HDR_OWNER_IP6,
	SDP_HDR_CONNECTION_IP6,
	SDP_HDR_MEDIA,
};

extern unsigned int (*nf_nat_sip_hook)(struct sk_buff *skb,
				       unsigned int dataoff,
				       const char **dptr,
				       unsigned int *datalen);
extern void (*nf_nat_sip_seq_adjust_hook)(struct sk_buff *skb, s16 off);
extern unsigned int (*nf_nat_sip_expect_hook)(struct sk_buff *skb,
					      unsigned int dataoff,
					      const char **dptr,
					      unsigned int *datalen,
					      struct nf_conntrack_expect *exp,
					      unsigned int matchoff,
					      unsigned int matchlen);
extern unsigned int (*nf_nat_sdp_addr_hook)(struct sk_buff *skb,
					    unsigned int dataoff,
					    const char **dptr,
					    unsigned int *datalen,
					    unsigned int sdpoff,
					    enum sdp_header_types type,
					    enum sdp_header_types term,
					    const union nf_inet_addr *addr);
extern unsigned int (*nf_nat_sdp_port_hook)(struct sk_buff *skb,
					    unsigned int dataoff,
					    const char **dptr,
					    unsigned int *datalen,
					    unsigned int matchoff,
					    unsigned int matchlen,
					    u_int16_t port);
extern unsigned int (*nf_nat_sdp_session_hook)(struct sk_buff *skb,
					       unsigned int dataoff,
					       const char **dptr,
					       unsigned int *datalen,
					       unsigned int sdpoff,
					       const union nf_inet_addr *addr);
extern unsigned int (*nf_nat_sdp_media_hook)(struct sk_buff *skb,
					     unsigned int dataoff,
					     const char **dptr,
					     unsigned int *datalen,
					     struct nf_conntrack_expect *rtp_exp,
					     struct nf_conntrack_expect *rtcp_exp,
					     unsigned int mediaoff,
					     unsigned int medialen,
					     union nf_inet_addr *rtp_addr);

extern int ct_sip_parse_request(const struct nf_conn *ct,
				const char *dptr, unsigned int datalen,
				unsigned int *matchoff, unsigned int *matchlen,
				union nf_inet_addr *addr, __be16 *port);
extern int ct_sip_get_header(const struct nf_conn *ct, const char *dptr,
			     unsigned int dataoff, unsigned int datalen,
			     enum sip_header_types type,
			     unsigned int *matchoff, unsigned int *matchlen);
extern int ct_sip_parse_header_uri(const struct nf_conn *ct, const char *dptr,
				   unsigned int *dataoff, unsigned int datalen,
				   enum sip_header_types type, int *in_header,
				   unsigned int *matchoff, unsigned int *matchlen,
				   union nf_inet_addr *addr, __be16 *port);
extern int ct_sip_parse_address_param(const struct nf_conn *ct, const char *dptr,
				      unsigned int dataoff, unsigned int datalen,
				      const char *name,
				      unsigned int *matchoff, unsigned int *matchlen,
				      union nf_inet_addr *addr);
extern int ct_sip_parse_numerical_param(const struct nf_conn *ct, const char *dptr,
					unsigned int off, unsigned int datalen,
					const char *name,
					unsigned int *matchoff, unsigned int *matchen,
					unsigned int *val);

extern int ct_sip_get_sdp_header(const struct nf_conn *ct, const char *dptr,
				 unsigned int dataoff, unsigned int datalen,
				 enum sdp_header_types type,
				 enum sdp_header_types term,
				 unsigned int *matchoff, unsigned int *matchlen);

#endif /* __KERNEL__ */
#endif /* __NF_CONNTRACK_SIP_H__ */