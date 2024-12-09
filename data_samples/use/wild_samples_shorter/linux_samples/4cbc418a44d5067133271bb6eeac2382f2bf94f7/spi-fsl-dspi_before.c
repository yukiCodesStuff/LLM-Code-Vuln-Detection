	struct spi_transfer			*cur_transfer;
	struct spi_message			*cur_msg;
	struct chip_data			*cur_chip;
	size_t					len;
	const void				*tx;
	void					*rx;
	void					*rx_end;
	dspi->tx_cmd |= SPI_PUSHR_CMD_CTCNT;

	if (dspi->devtype_data->xspi_mode && dspi->bits_per_word > 16) {
		/* Write two TX FIFO entries first, and then the corresponding
		 * CMD FIFO entry.
		 */
		u32 data = dspi_pop_tx(dspi);

		if (dspi->cur_chip->ctar_val & SPI_CTAR_LSBFE) {
			/* LSB */
			tx_fifo_write(dspi, data & 0xFFFF);
			tx_fifo_write(dspi, data >> 16);
		} else {
			/* MSB */
			tx_fifo_write(dspi, data >> 16);
			tx_fifo_write(dspi, data & 0xFFFF);
		}
		cmd_fifo_write(dspi);
	} else {
		/* Write one entry to both TX FIFO and CMD FIFO
		 * simultaneously.
		 */
	u32 spi_tcr;

	spi_take_timestamp_post(dspi->ctlr, dspi->cur_transfer,
				dspi->tx - dspi->bytes_per_word, !dspi->irq);

	/* Get transfer counter (in number of SPI transfers). It was
	 * reset to 0 when transfer(s) were started.
	 */
	spi_tcnt = SPI_TCR_GET_TCNT(spi_tcr);
	/* Update total number of bytes that were transferred */
	msg->actual_length += spi_tcnt * dspi->bytes_per_word;

	trans_mode = dspi->devtype_data->trans_mode;
	if (trans_mode == DSPI_EOQ_MODE)
		dspi_eoq_read(dspi);
		return 0;

	spi_take_timestamp_pre(dspi->ctlr, dspi->cur_transfer,
			       dspi->tx, !dspi->irq);

	if (trans_mode == DSPI_EOQ_MODE)
		dspi_eoq_write(dspi);
	else if (trans_mode == DSPI_TCFQ_MODE)
		dspi->rx = transfer->rx_buf;
		dspi->rx_end = dspi->rx + transfer->len;
		dspi->len = transfer->len;
		/* Validated transfer specific frame size (defaults applied) */
		dspi->bits_per_word = transfer->bits_per_word;
		if (transfer->bits_per_word <= 8)
			dspi->bytes_per_word = 1;
				     SPI_CTARE_DTCP(1));

		spi_take_timestamp_pre(dspi->ctlr, dspi->cur_transfer,
				       dspi->tx, !dspi->irq);

		trans_mode = dspi->devtype_data->trans_mode;
		switch (trans_mode) {
		case DSPI_EOQ_MODE: