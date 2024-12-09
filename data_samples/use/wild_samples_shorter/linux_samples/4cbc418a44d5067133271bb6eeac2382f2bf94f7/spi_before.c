 *			    advances its @tx buffer pointer monotonically.
 * @ctlr: Pointer to the spi_controller structure of the driver
 * @xfer: Pointer to the transfer being timestamped
 * @tx: Pointer to the current word within the xfer->tx_buf that the driver is
 *	preparing to transmit right now.
 * @irqs_off: If true, will disable IRQs and preemption for the duration of the
 *	      transfer, for less jitter in time measurement. Only compatible
 *	      with PIO drivers. If true, must follow up with
 *	      spi_take_timestamp_post or otherwise system will crash.
 */
void spi_take_timestamp_pre(struct spi_controller *ctlr,
			    struct spi_transfer *xfer,
			    const void *tx, bool irqs_off)
{
	u8 bytes_per_word = DIV_ROUND_UP(xfer->bits_per_word, 8);

	if (!xfer->ptp_sts)
		return;

	if (xfer->timestamped_pre)
		return;

	if (tx < (xfer->tx_buf + xfer->ptp_sts_word_pre * bytes_per_word))
		return;

	/* Capture the resolution of the timestamp */
	xfer->ptp_sts_word_pre = (tx - xfer->tx_buf) / bytes_per_word;

	xfer->timestamped_pre = true;

	if (irqs_off) {
 *			     timestamped.
 * @ctlr: Pointer to the spi_controller structure of the driver
 * @xfer: Pointer to the transfer being timestamped
 * @tx: Pointer to the current word within the xfer->tx_buf that the driver has
 *	just transmitted.
 * @irqs_off: If true, will re-enable IRQs and preemption for the local CPU.
 */
void spi_take_timestamp_post(struct spi_controller *ctlr,
			     struct spi_transfer *xfer,
			     const void *tx, bool irqs_off)
{
	u8 bytes_per_word = DIV_ROUND_UP(xfer->bits_per_word, 8);

	if (!xfer->ptp_sts)
		return;

	if (xfer->timestamped_post)
		return;

	if (tx < (xfer->tx_buf + xfer->ptp_sts_word_post * bytes_per_word))
		return;

	ptp_read_system_postts(xfer->ptp_sts);

	}

	/* Capture the resolution of the timestamp */
	xfer->ptp_sts_word_post = (tx - xfer->tx_buf) / bytes_per_word;

	xfer->timestamped_post = true;
}
EXPORT_SYMBOL_GPL(spi_take_timestamp_post);