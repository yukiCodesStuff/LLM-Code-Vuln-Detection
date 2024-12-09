	if (offset < VLAN_ETH_HLEN + vlan_hlen) {
		u8 ethlen = len;

		if (vlan_hlen &&
		    skb_copy_bits(skb, mac_off, &veth, VLAN_ETH_HLEN) < 0)
			return false;
		else if (!nft_payload_rebuild_vlan_hdr(skb, mac_off, &veth))
			return false;

		if (offset + len > VLAN_ETH_HLEN + vlan_hlen)
			ethlen -= offset + len - VLAN_ETH_HLEN + vlan_hlen;

		memcpy(dst_u8, vlanh + offset - vlan_hlen, ethlen);

		len -= ethlen;
		if (len == 0)
			return true;

		dst_u8 += ethlen;
		offset = ETH_HLEN + vlan_hlen;
	} else {
		offset -= VLAN_HLEN + vlan_hlen;
	}