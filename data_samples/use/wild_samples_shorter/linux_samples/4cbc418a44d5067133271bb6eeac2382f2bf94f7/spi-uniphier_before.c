	}
}

static void uniphier_spi_fill_tx_fifo(struct uniphier_spi_priv *priv)
{
	unsigned int fifo_threshold, fill_bytes;
	u32 val;

	fifo_threshold = DIV_ROUND_UP(priv->rx_bytes,
				bytes_per_word(priv->bits_per_word));
	fifo_threshold = min(fifo_threshold, SSI_FIFO_DEPTH);

	fill_bytes = fifo_threshold - (priv->rx_bytes - priv->tx_bytes);

	/* set fifo threshold */
	val = readl(priv->base + SSI_FC);
	val &= ~(SSI_FC_TXFTH_MASK | SSI_FC_RXFTH_MASK);
	val |= FIELD_PREP(SSI_FC_TXFTH_MASK, fifo_threshold);
	val |= FIELD_PREP(SSI_FC_RXFTH_MASK, fifo_threshold);
	writel(val, priv->base + SSI_FC);

	while (fill_bytes--)
		uniphier_spi_send(priv);
}

static void uniphier_spi_set_cs(struct spi_device *spi, bool enable)