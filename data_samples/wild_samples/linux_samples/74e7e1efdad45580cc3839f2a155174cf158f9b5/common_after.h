{
	return to_xenbus_device(vif->dev->dev.parent);
}

void xenvif_tx_credit_callback(struct timer_list *t);

struct xenvif *xenvif_alloc(struct device *parent,
			    domid_t domid,
			    unsigned int handle);

int xenvif_init_queue(struct xenvif_queue *queue);
void xenvif_deinit_queue(struct xenvif_queue *queue);

int xenvif_connect_data(struct xenvif_queue *queue,
			unsigned long tx_ring_ref,
			unsigned long rx_ring_ref,
			unsigned int tx_evtchn,
			unsigned int rx_evtchn);
void xenvif_disconnect_data(struct xenvif *vif);
int xenvif_connect_ctrl(struct xenvif *vif, grant_ref_t ring_ref,
			unsigned int evtchn);
void xenvif_disconnect_ctrl(struct xenvif *vif);
void xenvif_free(struct xenvif *vif);

int xenvif_xenbus_init(void);
void xenvif_xenbus_fini(void);

/* (Un)Map communication rings. */
void xenvif_unmap_frontend_data_rings(struct xenvif_queue *queue);
int xenvif_map_frontend_data_rings(struct xenvif_queue *queue,
				   grant_ref_t tx_ring_ref,
				   grant_ref_t rx_ring_ref);

/* Check for SKBs from frontend and schedule backend processing */
void xenvif_napi_schedule_or_enable_events(struct xenvif_queue *queue);

/* Prevent the device from generating any further traffic. */
void xenvif_carrier_off(struct xenvif *vif);

int xenvif_tx_action(struct xenvif_queue *queue, int budget);

int xenvif_kthread_guest_rx(void *data);
void xenvif_kick_thread(struct xenvif_queue *queue);

int xenvif_dealloc_kthread(void *data);

irqreturn_t xenvif_ctrl_irq_fn(int irq, void *data);

bool xenvif_have_rx_work(struct xenvif_queue *queue, bool test_kthread);
bool xenvif_rx_queue_tail(struct xenvif_queue *queue, struct sk_buff *skb);

void xenvif_carrier_on(struct xenvif *vif);

/* Callback from stack when TX packet can be released */
void xenvif_zerocopy_callback(struct sk_buff *skb, struct ubuf_info *ubuf,
			      bool zerocopy_success);

static inline pending_ring_idx_t nr_pending_reqs(struct xenvif_queue *queue)
{
	return MAX_PENDING_REQS -
		queue->pending_prod + queue->pending_cons;
}