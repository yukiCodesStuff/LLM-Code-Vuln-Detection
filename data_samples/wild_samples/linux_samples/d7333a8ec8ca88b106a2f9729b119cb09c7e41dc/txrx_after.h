struct wcn36xx_rx_bd {
	u32	bdt:2;
	u32	ft:1;
	u32	dpu_ne:1;
	u32	rx_key_id:3;
	u32	ub:1;
	u32	rmf:1;
	u32	uma_bypass:1;
	u32	csr11:1;
	u32	reserved0:1;
	u32	scan_learn:1;
	u32	rx_ch:4;
	u32	rtsf:1;
	u32	bsf:1;
	u32	a2hf:1;
	u32	st_auf:1;
	u32	dpu_sign:3;
	u32	dpu_rf:8;

	struct wcn36xx_pdu pdu;

	/* 0x14*/
	u32	addr3:8;
	u32	addr2:8;
	u32	addr1:8;
	u32	dpu_desc_idx:8;

	/* 0x18*/
	u32	rxp_flags:23;
	u32	rate_id:9;

	u32	phy_stat0;
	u32	phy_stat1;

	/* 0x24 */
	u32	rx_times;

	u32	pmi_cmd[6];

	/* 0x40 */
	u32	reserved7:4;
	u32	reorder_slot_id:6;
	u32	reorder_fwd_id:6;
	u32	reserved6:12;
	u32	reorder_code:4;

	/* 0x44 */
	u32	exp_seq_num:12;
	u32	cur_seq_num:12;
	u32	rf_band:2;
	u32	fr_type_subtype:6;

	/* 0x48 */
	u32	msdu_size:16;
	u32	sub_fr_id:4;
	u32	proc_order:4;
	u32	reserved9:4;
	u32	aef:1;
	u32	lsf:1;
	u32	esf:1;
	u32	asf:1;
};

struct wcn36xx_tx_bd {
	u32	bdt:2;
	u32	ft:1;
	u32	dpu_ne:1;
	u32	fw_tx_comp:1;
	u32	tx_comp:1;
	u32	reserved1:1;
	u32	ub:1;
	u32	rmf:1;
	u32	reserved0:12;
	u32	dpu_sign:3;
	u32	dpu_rf:8;

	struct wcn36xx_pdu pdu;

	/* 0x14*/
	u32	reserved5:7;
	u32	queue_id:5;
	u32	bd_rate:2;
	u32	ack_policy:2;
	u32	sta_index:8;
	u32	dpu_desc_idx:8;

	u32	tx_bd_sign;
	u32	reserved6;
	u32	dxe_start_time;
	u32	dxe_end_time;

	/*u32	tcp_udp_start_off:10;
	u32	header_cks:16;
	u32	reserved7:6;*/
};

struct wcn36xx_sta;
struct wcn36xx;

int  wcn36xx_rx_skb(struct wcn36xx *wcn, struct sk_buff *skb);
int wcn36xx_start_tx(struct wcn36xx *wcn,
		     struct wcn36xx_sta *sta_priv,
		     struct sk_buff *skb);

#endif	/* _TXRX_H_ */