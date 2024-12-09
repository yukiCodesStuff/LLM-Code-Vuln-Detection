{
	return to_xenbus_device(vif->dev->dev.parent);
}

#define XEN_NETIF_TX_RING_SIZE __CONST_RING_SIZE(xen_netif_tx, PAGE_SIZE)
#define XEN_NETIF_RX_RING_SIZE __CONST_RING_SIZE(xen_netif_rx, PAGE_SIZE)

struct xenvif *xenvif_alloc(struct device *parent,
			    domid_t domid,
			    unsigned int handle);

int xenvif_connect(struct xenvif *vif, unsigned long tx_ring_ref,
		   unsigned long rx_ring_ref, unsigned int evtchn);
void xenvif_disconnect(struct xenvif *vif);

void xenvif_get(struct xenvif *vif);
void xenvif_put(struct xenvif *vif);

int xenvif_xenbus_init(void);

int xenvif_schedulable(struct xenvif *vif);

int xen_netbk_rx_ring_full(struct xenvif *vif);

int xen_netbk_must_stop_queue(struct xenvif *vif);

/* (Un)Map communication rings. */
void xen_netbk_unmap_frontend_rings(struct xenvif *vif);
int xen_netbk_map_frontend_rings(struct xenvif *vif,
				 grant_ref_t tx_ring_ref,
				 grant_ref_t rx_ring_ref);

/* (De)Register a xenvif with the netback backend. */
void xen_netbk_add_xenvif(struct xenvif *vif);
void xen_netbk_remove_xenvif(struct xenvif *vif);

/* (De)Schedule backend processing for a xenvif */
void xen_netbk_schedule_xenvif(struct xenvif *vif);
void xen_netbk_deschedule_xenvif(struct xenvif *vif);

/* Check for SKBs from frontend and schedule backend processing */
void xen_netbk_check_rx_xenvif(struct xenvif *vif);
/* Receive an SKB from the frontend */
void xenvif_receive_skb(struct xenvif *vif, struct sk_buff *skb);

/* Queue an SKB for transmission to the frontend */
void xen_netbk_queue_tx_skb(struct xenvif *vif, struct sk_buff *skb);
/* Notify xenvif that ring now has space to send an skb to the frontend */
void xenvif_notify_tx_completion(struct xenvif *vif);

/* Prevent the device from generating any further traffic. */
void xenvif_carrier_off(struct xenvif *vif);

/* Returns number of ring slots required to send an skb to the frontend */
unsigned int xen_netbk_count_skb_slots(struct xenvif *vif, struct sk_buff *skb);

#endif /* __XEN_NETBACK__COMMON_H__ */