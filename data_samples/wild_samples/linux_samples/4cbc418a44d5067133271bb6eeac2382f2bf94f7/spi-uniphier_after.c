		switch (rsize) {
		case 1:
			*priv->rx_buf = val;
			break;
		case 2:
			put_unaligned_le16(val, priv->rx_buf);
			break;
		case 4:
			put_unaligned_le32(val, priv->rx_buf);
			break;
		}

		priv->rx_buf += rsize;
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
{
	struct uniphier_spi_priv *priv = spi_master_get_devdata(spi->master);
	u32 val;

	val = readl(priv->base + SSI_FPS);

	if (enable)
		val |= SSI_FPS_FSPOL;
	else
		val &= ~SSI_FPS_FSPOL;

	writel(val, priv->base + SSI_FPS);
}

static int uniphier_spi_transfer_one_irq(struct spi_master *master,
					 struct spi_device *spi,
					 struct spi_transfer *t)
{
	struct uniphier_spi_priv *priv = spi_master_get_devdata(master);
	struct device *dev = master->dev.parent;
	unsigned long time_left;

	reinit_completion(&priv->xfer_done);

	uniphier_spi_fill_tx_fifo(priv);

	uniphier_spi_irq_enable(spi, SSI_IE_RCIE | SSI_IE_RORIE);

	time_left = wait_for_completion_timeout(&priv->xfer_done,
					msecs_to_jiffies(SSI_TIMEOUT_MS));

	uniphier_spi_irq_disable(spi, SSI_IE_RCIE | SSI_IE_RORIE);

	if (!time_left) {
		dev_err(dev, "transfer timeout.\n");
		return -ETIMEDOUT;
	}