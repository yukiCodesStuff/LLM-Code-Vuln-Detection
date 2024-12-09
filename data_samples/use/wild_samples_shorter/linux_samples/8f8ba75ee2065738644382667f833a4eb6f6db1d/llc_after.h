extern void llc_sap_handler(struct llc_sap *sap, struct sk_buff *skb);
extern void llc_conn_handler(struct llc_sap *sap, struct sk_buff *skb);

extern void llc_station_init(void);
extern void llc_station_exit(void);

#ifdef CONFIG_PROC_FS
extern int llc_proc_init(void);