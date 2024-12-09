				    (rss_hash & BIT(0))) {
					encap = true;
					outer_csum_ok = (rss_hash & BIT(1)) &&
							(rss_hash & BIT(2));
				}
				break;
			}
		}

		/* Hardware does not provide whole packet checksum. It only
		 * provides pseudo checksum. Since hw validates the packet
		 * checksum but not provide us the checksum value. use
		 * CHECSUM_UNNECESSARY.
		 *
		 * In case of encap pkt tcp_udp_csum_ok/tcp_udp_csum_ok is
		 * inner csum_ok. outer_csum_ok is set by hw when outer udp
		 * csum is correct or is zero.
		 */
		if ((netdev->features & NETIF_F_RXCSUM) && !csum_not_calc &&
		    tcp_udp_csum_ok && outer_csum_ok &&
		    (ipv4_csum_ok || ipv6)) {
			skb->ip_summed = CHECKSUM_UNNECESSARY;
			skb->csum_level = encap;
		}