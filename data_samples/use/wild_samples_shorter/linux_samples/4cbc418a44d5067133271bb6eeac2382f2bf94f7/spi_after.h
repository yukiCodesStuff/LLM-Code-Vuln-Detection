/* Helper calls for driver to timestamp transfer */
void spi_take_timestamp_pre(struct spi_controller *ctlr,
			    struct spi_transfer *xfer,
			    size_t progress, bool irqs_off);
void spi_take_timestamp_post(struct spi_controller *ctlr,
			     struct spi_transfer *xfer,
			     size_t progress, bool irqs_off);

/* the spi driver core manages memory for the spi_controller classdev */
extern struct spi_controller *__spi_alloc_controller(struct device *host,
						unsigned int size, bool slave);