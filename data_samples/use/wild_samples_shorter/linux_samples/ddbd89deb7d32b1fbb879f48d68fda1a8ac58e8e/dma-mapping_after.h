 */
#define DMA_ATTR_PRIVILEGED		(1UL << 9)

/*
 * This is a hint to the DMA-mapping subsystem that the device is expected
 * to overwrite the entire mapped size, thus the caller does not require any
 * of the previous buffer contents to be preserved. This allows
 * bounce-buffering implementations to optimise DMA_FROM_DEVICE transfers.
 */
#define DMA_ATTR_OVERWRITE		(1UL << 10)

/*
 * A dma_addr_t can hold any valid DMA or bus address for the platform.  It can
 * be given to a device to use as a DMA source or target.  It is specific to a
 * given device and there may be a translation between the CPU physical address