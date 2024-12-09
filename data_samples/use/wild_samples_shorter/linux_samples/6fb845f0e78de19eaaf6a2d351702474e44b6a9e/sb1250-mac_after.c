		 * for transmits, we just free buffers.
		 */

		dev_consume_skb_irq(sb);

		/*
		 * .. and advance to the next buffer.
		 */