		tx_level -= db->rptr->len;	/* '-' koz len is negative */

		/* now should come skb pointer - free it */
		dev_kfree_skb_irq(db->rptr->addr.skb);
		bdx_tx_db_inc_rptr(db);
	}

	/* let h/w know which TXF descriptors were cleaned */