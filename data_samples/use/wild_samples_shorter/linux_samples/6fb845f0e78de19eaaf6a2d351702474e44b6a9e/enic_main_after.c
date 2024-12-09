		 * csum is correct or is zero.
		 */
		if ((netdev->features & NETIF_F_RXCSUM) && !csum_not_calc &&
		    tcp_udp_csum_ok && outer_csum_ok &&
		    (ipv4_csum_ok || ipv6)) {
			skb->ip_summed = CHECKSUM_UNNECESSARY;
			skb->csum_level = encap;
		}
