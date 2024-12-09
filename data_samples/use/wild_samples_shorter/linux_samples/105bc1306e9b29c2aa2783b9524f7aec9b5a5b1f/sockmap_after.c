		if (i == MAX_SKB_FRAGS)
			i = 0;
	}
	consume_skb(md->skb);

	return free;
}


		if (!sg->length && md->sg_start == md->sg_end) {
			list_del(&md->list);
			consume_skb(md->skb);
			kfree(md);
		}
	}
