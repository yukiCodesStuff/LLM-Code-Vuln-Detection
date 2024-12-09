			return false;

		if (offset + len > VLAN_ETH_HLEN + vlan_hlen)
			ethlen -= offset + len - VLAN_ETH_HLEN - vlan_hlen;

		memcpy(dst_u8, vlanh + offset - vlan_hlen, ethlen);

		len -= ethlen;