}

static struct nft_set *nft_set_lookup_byid(const struct net *net,
					   const struct nlattr *nla, u8 genmask)
{
	struct nftables_pernet *nft_net = nft_pernet(net);
	u32 id = ntohl(nla_get_be32(nla));
			struct nft_set *set = nft_trans_set(trans);

			if (id == nft_trans_set_id(trans) &&
			    nft_active_genmask(set, genmask))
				return set;
		}
	}
		if (!nla_set_id)
			return set;

		set = nft_set_lookup_byid(net, nla_set_id, genmask);
	}
	return set;
}
EXPORT_SYMBOL_GPL(nft_set_lookup_global);