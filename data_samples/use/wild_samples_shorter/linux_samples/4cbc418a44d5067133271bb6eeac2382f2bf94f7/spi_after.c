 *			    advances its @tx buffer pointer monotonically.
 * @ctlr: Pointer to the spi_controller structure of the driver
 * @xfer: Pointer to the transfer being timestamped
 * @progress: How many words (not bytes) have been transferred so far
 * @irqs_off: If true, will disable IRQs and preemption for the duration of the
 *	      transfer, for less jitter in time measurement. Only compatible
 *	      with PIO drivers. If true, must follow up with
 *	      spi_take_timestamp_post or otherwise system will crash.
 */
void spi_take_timestamp_pre(struct spi_controller *ctlr,
			    struct spi_transfer *xfer,
			    size_t progress, bool irqs_off)
{
	if (!xfer->ptp_sts)
		return;

	if (xfer->timestamped_pre)
		return;

	if (progress < xfer->ptp_sts_word_pre)
		return;

	/* Capture the resolution of the timestamp */
	xfer->ptp_sts_word_pre = progress;

	xfer->timestamped_pre = true;

	if (irqs_off) {
 *			     timestamped.
 * @ctlr: Pointer to the spi_controller structure of the driver
 * @xfer: Pointer to the transfer being timestamped
 * @progress: How many words (not bytes) have been transferred so far
 * @irqs_off: If true, will re-enable IRQs and preemption for the local CPU.
 */
void spi_take_timestamp_post(struct spi_controller *ctlr,
			     struct spi_transfer *xfer,
			     size_t progress, bool irqs_off)
{
	if (!xfer->ptp_sts)
		return;

	if (xfer->timestamped_post)
		return;

	if (progress < xfer->ptp_sts_word_post)
		return;

	ptp_read_system_postts(xfer->ptp_sts);

	}

	/* Capture the resolution of the timestamp */
	xfer->ptp_sts_word_post = progress;

	xfer->timestamped_post = true;
}
EXPORT_SYMBOL_GPL(spi_take_timestamp_post);