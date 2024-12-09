{
	if (atomic_dec_and_test(&sap->refcnt))
		llc_sap_close(sap);
}

extern struct llc_sap *llc_sap_find(unsigned char sap_value);

extern int llc_build_and_send_ui_pkt(struct llc_sap *sap, struct sk_buff *skb,
				     unsigned char *dmac, unsigned char dsap);

extern void llc_sap_handler(struct llc_sap *sap, struct sk_buff *skb);
extern void llc_conn_handler(struct llc_sap *sap, struct sk_buff *skb);

extern void llc_station_init(void);
extern void llc_station_exit(void);

#ifdef CONFIG_PROC_FS
extern int llc_proc_init(void);
extern void llc_proc_exit(void);
#else
#define llc_proc_init()	(0)
#define llc_proc_exit()	do { } while(0)
#endif /* CONFIG_PROC_FS */
#ifdef CONFIG_SYSCTL
extern int llc_sysctl_init(void);
extern void llc_sysctl_exit(void);

extern int sysctl_llc2_ack_timeout;
extern int sysctl_llc2_busy_timeout;
extern int sysctl_llc2_p_timeout;
extern int sysctl_llc2_rej_timeout;
extern int sysctl_llc_station_ack_timeout;
#else
#define llc_sysctl_init() (0)
#define llc_sysctl_exit() do { } while(0)
#endif /* CONFIG_SYSCTL */
#endif /* LLC_H */