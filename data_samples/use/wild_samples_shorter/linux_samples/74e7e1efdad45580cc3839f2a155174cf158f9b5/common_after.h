irqreturn_t xenvif_ctrl_irq_fn(int irq, void *data);

bool xenvif_have_rx_work(struct xenvif_queue *queue, bool test_kthread);
bool xenvif_rx_queue_tail(struct xenvif_queue *queue, struct sk_buff *skb);

void xenvif_carrier_on(struct xenvif *vif);

/* Callback from stack when TX packet can be released */