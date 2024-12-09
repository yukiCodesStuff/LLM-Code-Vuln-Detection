		 * for transmits, we just free buffers.
		 */

		dev_kfree_skb_irq(sb);

		/*
		 * .. and advance to the next buffer.
		 */