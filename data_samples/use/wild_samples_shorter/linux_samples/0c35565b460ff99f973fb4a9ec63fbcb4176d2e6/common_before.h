/* Notify xenvif that ring now has space to send an skb to the frontend */
void xenvif_notify_tx_completion(struct xenvif *vif);

/* Returns number of ring slots required to send an skb to the frontend */
unsigned int xen_netbk_count_skb_slots(struct xenvif *vif, struct sk_buff *skb);

#endif /* __XEN_NETBACK__COMMON_H__ */