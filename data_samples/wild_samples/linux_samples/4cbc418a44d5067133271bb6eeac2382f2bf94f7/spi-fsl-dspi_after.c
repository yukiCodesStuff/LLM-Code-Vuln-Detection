struct fsl_dspi {
	struct spi_controller			*ctlr;
	struct platform_device			*pdev;

	struct regmap				*regmap;
	struct regmap				*regmap_pushr;
	int					irq;
	struct clk				*clk;

	struct spi_transfer			*cur_transfer;
	struct spi_message			*cur_msg;
	struct chip_data			*cur_chip;
	size_t					progress;
	size_t					len;
	const void				*tx;
	void					*rx;
	void					*rx_end;
	u16					void_write_data;
	u16					tx_cmd;
	u8					bits_per_word;
	u8					bytes_per_word;
	const struct fsl_dspi_devtype_data	*devtype_data;

	wait_queue_head_t			waitq;
	u32					waitflags;

	struct fsl_dspi_dma			*dma;
};

static u32 dspi_pop_tx(struct fsl_dspi *dspi)
{
	u32 txdata = 0;

	if (dspi->tx) {
		if (dspi->bytes_per_word == 1)
			txdata = *(u8 *)dspi->tx;
		else if (dspi->bytes_per_word == 2)
			txdata = *(u16 *)dspi->tx;
		else  /* dspi->bytes_per_word == 4 */
			txdata = *(u32 *)dspi->tx;
		dspi->tx += dspi->bytes_per_word;
	}
	dspi->len -= dspi->bytes_per_word;
	return txdata;
}
{
	/* Clear transfer count */
	dspi->tx_cmd |= SPI_PUSHR_CMD_CTCNT;

	if (dspi->devtype_data->xspi_mode && dspi->bits_per_word > 16) {
		/* Write the CMD FIFO entry first, and then the two
		 * corresponding TX FIFO entries.
		 */
		u32 data = dspi_pop_tx(dspi);

		cmd_fifo_write(dspi);
		tx_fifo_write(dspi, data & 0xFFFF);
		tx_fifo_write(dspi, data >> 16);
	} else {
		/* Write one entry to both TX FIFO and CMD FIFO
		 * simultaneously.
		 */
		fifo_write(dspi);
	}
}
{
	struct spi_message *msg = dspi->cur_msg;
	enum dspi_trans_mode trans_mode;
	u16 spi_tcnt;
	u32 spi_tcr;

	spi_take_timestamp_post(dspi->ctlr, dspi->cur_transfer,
				dspi->progress, !dspi->irq);

	/* Get transfer counter (in number of SPI transfers). It was
	 * reset to 0 when transfer(s) were started.
	 */
	regmap_read(dspi->regmap, SPI_TCR, &spi_tcr);
	spi_tcnt = SPI_TCR_GET_TCNT(spi_tcr);
	/* Update total number of bytes that were transferred */
	msg->actual_length += spi_tcnt * dspi->bytes_per_word;
	dspi->progress += spi_tcnt;

	trans_mode = dspi->devtype_data->trans_mode;
	if (trans_mode == DSPI_EOQ_MODE)
		dspi_eoq_read(dspi);
	else if (trans_mode == DSPI_TCFQ_MODE)
		dspi_tcfq_read(dspi);

	if (!dspi->len)
		/* Success! */
		return 0;

	spi_take_timestamp_pre(dspi->ctlr, dspi->cur_transfer,
			       dspi->progress, !dspi->irq);

	if (trans_mode == DSPI_EOQ_MODE)
		dspi_eoq_write(dspi);
	else if (trans_mode == DSPI_TCFQ_MODE)
		dspi_tcfq_write(dspi);

	return -EINPROGRESS;
}

static int dspi_poll(struct fsl_dspi *dspi)
{
	int tries = 1000;
	u32 spi_sr;

	do {
		regmap_read(dspi->regmap, SPI_SR, &spi_sr);
		regmap_write(dspi->regmap, SPI_SR, spi_sr);

		if (spi_sr & (SPI_SR_EOQF | SPI_SR_TCFQF))
			break;
	} while (--tries);

	if (!tries)
		return -ETIMEDOUT;

	return dspi_rxtx(dspi);
}
{
	struct spi_message *msg = dspi->cur_msg;
	enum dspi_trans_mode trans_mode;
	u16 spi_tcnt;
	u32 spi_tcr;

	spi_take_timestamp_post(dspi->ctlr, dspi->cur_transfer,
				dspi->progress, !dspi->irq);

	/* Get transfer counter (in number of SPI transfers). It was
	 * reset to 0 when transfer(s) were started.
	 */
	regmap_read(dspi->regmap, SPI_TCR, &spi_tcr);
	spi_tcnt = SPI_TCR_GET_TCNT(spi_tcr);
	/* Update total number of bytes that were transferred */
	msg->actual_length += spi_tcnt * dspi->bytes_per_word;
	dspi->progress += spi_tcnt;

	trans_mode = dspi->devtype_data->trans_mode;
	if (trans_mode == DSPI_EOQ_MODE)
		dspi_eoq_read(dspi);
	else if (trans_mode == DSPI_TCFQ_MODE)
		dspi_tcfq_read(dspi);

	if (!dspi->len)
		/* Success! */
		return 0;

	spi_take_timestamp_pre(dspi->ctlr, dspi->cur_transfer,
			       dspi->progress, !dspi->irq);

	if (trans_mode == DSPI_EOQ_MODE)
		dspi_eoq_write(dspi);
	else if (trans_mode == DSPI_TCFQ_MODE)
		dspi_tcfq_write(dspi);

	return -EINPROGRESS;
}

static int dspi_poll(struct fsl_dspi *dspi)
{
	int tries = 1000;
	u32 spi_sr;

	do {
		regmap_read(dspi->regmap, SPI_SR, &spi_sr);
		regmap_write(dspi->regmap, SPI_SR, spi_sr);

		if (spi_sr & (SPI_SR_EOQF | SPI_SR_TCFQF))
			break;
	} while (--tries);

	if (!tries)
		return -ETIMEDOUT;

	return dspi_rxtx(dspi);
}
{
	struct spi_message *msg = dspi->cur_msg;
	enum dspi_trans_mode trans_mode;
	u16 spi_tcnt;
	u32 spi_tcr;

	spi_take_timestamp_post(dspi->ctlr, dspi->cur_transfer,
				dspi->progress, !dspi->irq);

	/* Get transfer counter (in number of SPI transfers). It was
	 * reset to 0 when transfer(s) were started.
	 */
	regmap_read(dspi->regmap, SPI_TCR, &spi_tcr);
	spi_tcnt = SPI_TCR_GET_TCNT(spi_tcr);
	/* Update total number of bytes that were transferred */
	msg->actual_length += spi_tcnt * dspi->bytes_per_word;
	dspi->progress += spi_tcnt;

	trans_mode = dspi->devtype_data->trans_mode;
	if (trans_mode == DSPI_EOQ_MODE)
		dspi_eoq_read(dspi);
	else if (trans_mode == DSPI_TCFQ_MODE)
		dspi_tcfq_read(dspi);

	if (!dspi->len)
		/* Success! */
		return 0;

	spi_take_timestamp_pre(dspi->ctlr, dspi->cur_transfer,
			       dspi->progress, !dspi->irq);

	if (trans_mode == DSPI_EOQ_MODE)
		dspi_eoq_write(dspi);
	else if (trans_mode == DSPI_TCFQ_MODE)
		dspi_tcfq_write(dspi);

	return -EINPROGRESS;
}

static int dspi_poll(struct fsl_dspi *dspi)
{
	int tries = 1000;
	u32 spi_sr;

	do {
		regmap_read(dspi->regmap, SPI_SR, &spi_sr);
		regmap_write(dspi->regmap, SPI_SR, spi_sr);

		if (spi_sr & (SPI_SR_EOQF | SPI_SR_TCFQF))
			break;
	} while (--tries);

	if (!tries)
		return -ETIMEDOUT;

	return dspi_rxtx(dspi);
}
		} else {
			/* Keep PCS active between transfers in same message
			 * when cs_change is not set, and de-activate PCS
			 * between transfers in the same message when
			 * cs_change is set.
			 */
			if (!transfer->cs_change)
				dspi->tx_cmd |= SPI_PUSHR_CMD_CONT;
		}

		dspi->void_write_data = dspi->cur_chip->void_write_data;

		dspi->tx = transfer->tx_buf;
		dspi->rx = transfer->rx_buf;
		dspi->rx_end = dspi->rx + transfer->len;
		dspi->len = transfer->len;
		dspi->progress = 0;
		/* Validated transfer specific frame size (defaults applied) */
		dspi->bits_per_word = transfer->bits_per_word;
		if (transfer->bits_per_word <= 8)
			dspi->bytes_per_word = 1;
		else if (transfer->bits_per_word <= 16)
			dspi->bytes_per_word = 2;
		else
			dspi->bytes_per_word = 4;

		regmap_update_bits(dspi->regmap, SPI_MCR,
				   SPI_MCR_CLR_TXF | SPI_MCR_CLR_RXF,
				   SPI_MCR_CLR_TXF | SPI_MCR_CLR_RXF);
		regmap_write(dspi->regmap, SPI_CTAR(0),
			     dspi->cur_chip->ctar_val |
			     SPI_FRAME_BITS(transfer->bits_per_word));
		if (dspi->devtype_data->xspi_mode)
			regmap_write(dspi->regmap, SPI_CTARE(0),
				     SPI_FRAME_EBITS(transfer->bits_per_word) |
				     SPI_CTARE_DTCP(1));

		spi_take_timestamp_pre(dspi->ctlr, dspi->cur_transfer,
				       dspi->progress, !dspi->irq);

		trans_mode = dspi->devtype_data->trans_mode;
		switch (trans_mode) {
		case DSPI_EOQ_MODE:
			regmap_write(dspi->regmap, SPI_RSER, SPI_RSER_EOQFE);
			dspi_eoq_write(dspi);
			break;
		case DSPI_TCFQ_MODE:
			regmap_write(dspi->regmap, SPI_RSER, SPI_RSER_TCFQE);
			dspi_tcfq_write(dspi);
			break;
		case DSPI_DMA_MODE:
			regmap_write(dspi->regmap, SPI_RSER,
				     SPI_RSER_TFFFE | SPI_RSER_TFFFD |
				     SPI_RSER_RFDFE | SPI_RSER_RFDFD);
			status = dspi_dma_xfer(dspi);
			break;
		default:
			dev_err(&dspi->pdev->dev, "unsupported trans_mode %u\n",
				trans_mode);
			status = -EINVAL;
			goto out;
		}
		} else {
			/* Keep PCS active between transfers in same message
			 * when cs_change is not set, and de-activate PCS
			 * between transfers in the same message when
			 * cs_change is set.
			 */
			if (!transfer->cs_change)
				dspi->tx_cmd |= SPI_PUSHR_CMD_CONT;
		}

		dspi->void_write_data = dspi->cur_chip->void_write_data;

		dspi->tx = transfer->tx_buf;
		dspi->rx = transfer->rx_buf;
		dspi->rx_end = dspi->rx + transfer->len;
		dspi->len = transfer->len;
		dspi->progress = 0;
		/* Validated transfer specific frame size (defaults applied) */
		dspi->bits_per_word = transfer->bits_per_word;
		if (transfer->bits_per_word <= 8)
			dspi->bytes_per_word = 1;
		else if (transfer->bits_per_word <= 16)
			dspi->bytes_per_word = 2;
		else
			dspi->bytes_per_word = 4;

		regmap_update_bits(dspi->regmap, SPI_MCR,
				   SPI_MCR_CLR_TXF | SPI_MCR_CLR_RXF,
				   SPI_MCR_CLR_TXF | SPI_MCR_CLR_RXF);
		regmap_write(dspi->regmap, SPI_CTAR(0),
			     dspi->cur_chip->ctar_val |
			     SPI_FRAME_BITS(transfer->bits_per_word));
		if (dspi->devtype_data->xspi_mode)
			regmap_write(dspi->regmap, SPI_CTARE(0),
				     SPI_FRAME_EBITS(transfer->bits_per_word) |
				     SPI_CTARE_DTCP(1));

		spi_take_timestamp_pre(dspi->ctlr, dspi->cur_transfer,
				       dspi->progress, !dspi->irq);

		trans_mode = dspi->devtype_data->trans_mode;
		switch (trans_mode) {
		case DSPI_EOQ_MODE:
			regmap_write(dspi->regmap, SPI_RSER, SPI_RSER_EOQFE);
			dspi_eoq_write(dspi);
			break;
		case DSPI_TCFQ_MODE:
			regmap_write(dspi->regmap, SPI_RSER, SPI_RSER_TCFQE);
			dspi_tcfq_write(dspi);
			break;
		case DSPI_DMA_MODE:
			regmap_write(dspi->regmap, SPI_RSER,
				     SPI_RSER_TFFFE | SPI_RSER_TFFFD |
				     SPI_RSER_RFDFE | SPI_RSER_RFDFD);
			status = dspi_dma_xfer(dspi);
			break;
		default:
			dev_err(&dspi->pdev->dev, "unsupported trans_mode %u\n",
				trans_mode);
			status = -EINVAL;
			goto out;
		}