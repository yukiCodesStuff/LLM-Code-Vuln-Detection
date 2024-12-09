efx_enqueue_skb(struct efx_tx_queue *tx_queue, struct sk_buff *skb);
extern void efx_xmit_done(struct efx_tx_queue *tx_queue, unsigned int index);
extern int efx_setup_tc(struct net_device *net_dev, u8 num_tc);

/* RX */
extern int efx_probe_rx_queue(struct efx_rx_queue *rx_queue);
extern void efx_remove_rx_queue(struct efx_rx_queue *rx_queue);
#define EFX_MAX_EVQ_SIZE 16384UL
#define EFX_MIN_EVQ_SIZE 512UL

/* The smallest [rt]xq_entries that the driver supports. Callers of
 * efx_wake_queue() assume that they can subsequently send at least one
 * skb. Falcon/A1 may require up to three descriptors per skb_frag. */
#define EFX_MIN_RING_SIZE (roundup_pow_of_two(2 * 3 * MAX_SKB_FRAGS))

/* Filters */
extern int efx_probe_filters(struct efx_nic *efx);
extern void efx_restore_filters(struct efx_nic *efx);