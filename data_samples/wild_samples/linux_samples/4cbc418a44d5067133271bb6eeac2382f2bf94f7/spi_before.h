{
	return IS_ENABLED(CONFIG_SPI_SLAVE) && ctlr->slave;
}

/* PM calls that need to be issued by the driver */
extern int spi_controller_suspend(struct spi_controller *ctlr);
extern int spi_controller_resume(struct spi_controller *ctlr);

/* Calls the driver make to interact with the message queue */
extern struct spi_message *spi_get_next_queued_message(struct spi_controller *ctlr);
extern void spi_finalize_current_message(struct spi_controller *ctlr);
extern void spi_finalize_current_transfer(struct spi_controller *ctlr);

/* Helper calls for driver to timestamp transfer */
void spi_take_timestamp_pre(struct spi_controller *ctlr,
			    struct spi_transfer *xfer,
			    const void *tx, bool irqs_off);
void spi_take_timestamp_post(struct spi_controller *ctlr,
			     struct spi_transfer *xfer,
			     const void *tx, bool irqs_off);

/* the spi driver core manages memory for the spi_controller classdev */
extern struct spi_controller *__spi_alloc_controller(struct device *host,
						unsigned int size, bool slave);

static inline struct spi_controller *spi_alloc_master(struct device *host,
						      unsigned int size)
{
	return __spi_alloc_controller(host, size, false);
}