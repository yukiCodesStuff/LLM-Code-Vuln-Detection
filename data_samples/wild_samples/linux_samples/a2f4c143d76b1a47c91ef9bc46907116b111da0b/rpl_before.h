{
	return 0;
}

static inline void rpl_exit(void) {}
#endif

/* Worst decompression memory usage ipv6 address (16) + pad 7 */
#define IPV6_RPL_SRH_WORST_SWAP_SIZE (sizeof(struct in6_addr) + 7)

size_t ipv6_rpl_srh_size(unsigned char n, unsigned char cmpri,
			 unsigned char cmpre);

void ipv6_rpl_srh_decompress(struct ipv6_rpl_sr_hdr *outhdr,
			     const struct ipv6_rpl_sr_hdr *inhdr,
			     const struct in6_addr *daddr, unsigned char n);

void ipv6_rpl_srh_compress(struct ipv6_rpl_sr_hdr *outhdr,
			   const struct ipv6_rpl_sr_hdr *inhdr,
			   const struct in6_addr *daddr, unsigned char n);

#endif /* _NET_RPL_H */