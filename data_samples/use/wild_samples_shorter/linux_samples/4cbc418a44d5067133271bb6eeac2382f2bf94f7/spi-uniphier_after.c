	}
}

static void uniphier_spi_set_fifo_threshold(struct uniphier_spi_priv *priv,
					    unsigned int threshold)
{
	u32 val;

	val = readl(priv->base + SSI_FC);
	val &= ~(SSI_FC_TXFTH_MASK | SSI_FC_RXFTH_MASK);
	val |= FIELD_PREP(SSI_FC_TXFTH_MASK, SSI_FIFO_DEPTH - threshold);
	val |= FIELD_PREP(SSI_FC_RXFTH_MASK, threshold);
	writel(val, priv->base + SSI_FC);
}

static void uniphier_spi_fill_tx_fifo(struct uniphier_spi_priv *priv)
{
	unsigned int fifo_threshold, fill_words;
	unsigned int bpw = bytes_per_word(priv->bits_per_word);

	fifo_threshold = DIV_ROUND_UP(priv->rx_bytes, bpw);
	fifo_threshold = min(fifo_threshold, SSI_FIFO_DEPTH);

	uniphier_spi_set_fifo_threshold(priv, fifo_threshold);

	fill_words = fifo_threshold -
		DIV_ROUND_UP(priv->rx_bytes - priv->tx_bytes, bpw);

	while (fill_words--)
		uniphier_spi_send(priv);
}

static void uniphier_spi_set_cs(struct spi_device *spi, bool enable)