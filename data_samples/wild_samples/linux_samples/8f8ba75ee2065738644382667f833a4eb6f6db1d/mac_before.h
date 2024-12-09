struct ath_tx_info {
	u8 qcu;

	bool is_first;
	bool is_last;

	enum aggr_type aggr;
	u8 ndelim;
	u16 aggr_len;

	dma_addr_t link;
	int pkt_len;
	u32 flags;

	dma_addr_t buf_addr[4];
	int buf_len[4];

	struct ath9k_11n_rate_series rates[4];
	u8 rtscts_rate;
	bool dur_update;

	enum ath9k_pkt_type type;
	enum ath9k_key_type keytype;
	u8 keyix;
	u8 txpower;
};

struct ath_hw;
struct ath9k_channel;
enum ath9k_int;

u32 ath9k_hw_gettxbuf(struct ath_hw *ah, u32 q);
void ath9k_hw_puttxbuf(struct ath_hw *ah, u32 q, u32 txdp);
void ath9k_hw_txstart(struct ath_hw *ah, u32 q);
u32 ath9k_hw_numtxpending(struct ath_hw *ah, u32 q);
bool ath9k_hw_updatetxtriglevel(struct ath_hw *ah, bool bIncTrigLevel);
bool ath9k_hw_stop_dma_queue(struct ath_hw *ah, u32 q);
void ath9k_hw_abort_tx_dma(struct ath_hw *ah);
bool ath9k_hw_set_txq_props(struct ath_hw *ah, int q,
			    const struct ath9k_tx_queue_info *qinfo);
bool ath9k_hw_get_txq_props(struct ath_hw *ah, int q,
			    struct ath9k_tx_queue_info *qinfo);
int ath9k_hw_setuptxqueue(struct ath_hw *ah, enum ath9k_tx_queue type,
			  const struct ath9k_tx_queue_info *qinfo);
bool ath9k_hw_releasetxqueue(struct ath_hw *ah, u32 q);
bool ath9k_hw_resettxqueue(struct ath_hw *ah, u32 q);
int ath9k_hw_rxprocdesc(struct ath_hw *ah, struct ath_desc *ds,
			struct ath_rx_status *rs);
void ath9k_hw_setuprxdesc(struct ath_hw *ah, struct ath_desc *ds,
			  u32 size, u32 flags);
bool ath9k_hw_setrxabort(struct ath_hw *ah, bool set);
void ath9k_hw_putrxbuf(struct ath_hw *ah, u32 rxdp);
void ath9k_hw_startpcureceive(struct ath_hw *ah, bool is_scanning);
void ath9k_hw_abortpcurecv(struct ath_hw *ah);
bool ath9k_hw_stopdmarecv(struct ath_hw *ah, bool *reset);
int ath9k_hw_beaconq_setup(struct ath_hw *ah);

/* Interrupt Handling */
bool ath9k_hw_intrpend(struct ath_hw *ah);
void ath9k_hw_set_interrupts(struct ath_hw *ah);
void ath9k_hw_enable_interrupts(struct ath_hw *ah);
void ath9k_hw_disable_interrupts(struct ath_hw *ah);

void ar9002_hw_attach_mac_ops(struct ath_hw *ah);

#endif /* MAC_H */