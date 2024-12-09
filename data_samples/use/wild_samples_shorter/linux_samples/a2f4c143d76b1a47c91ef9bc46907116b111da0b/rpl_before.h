static inline void rpl_exit(void) {}
#endif

/* Worst decompression memory usage ipv6 address (16) + pad 7 */
#define IPV6_RPL_SRH_WORST_SWAP_SIZE (sizeof(struct in6_addr) + 7)

size_t ipv6_rpl_srh_size(unsigned char n, unsigned char cmpri,
			 unsigned char cmpre);

void ipv6_rpl_srh_decompress(struct ipv6_rpl_sr_hdr *outhdr,