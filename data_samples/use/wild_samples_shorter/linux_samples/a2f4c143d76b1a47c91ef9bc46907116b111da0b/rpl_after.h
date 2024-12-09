static inline void rpl_exit(void) {}
#endif

size_t ipv6_rpl_srh_size(unsigned char n, unsigned char cmpri,
			 unsigned char cmpre);

void ipv6_rpl_srh_decompress(struct ipv6_rpl_sr_hdr *outhdr,