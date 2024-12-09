struct ucc_geth_hardware_statistics {
	u32 tx64;		/* Total number of frames (including bad
				   frames) transmitted that were exactly of the
				   minimal length (64 for un tagged, 68 for
				   tagged, or with length exactly equal to the
				   parameter MINLength */
	u32 tx127;		/* Total number of frames (including bad
				   frames) transmitted that were between
				   MINLength (Including FCS length==4) and 127
				   octets */
	u32 tx255;		/* Total number of frames (including bad
				   frames) transmitted that were between 128
				   (Including FCS length==4) and 255 octets */
	u32 rx64;		/* Total number of frames received including
				   bad frames that were exactly of the mninimal
				   length (64 bytes) */
	u32 rx127;		/* Total number of frames (including bad
				   frames) received that were between MINLength
				   (Including FCS length==4) and 127 octets */
	u32 rx255;		/* Total number of frames (including bad
				   frames) received that were between 128
				   (Including FCS length==4) and 255 octets */
	u32 txok;		/* Total number of octets residing in frames
				   that where involved in successfull
				   transmission */
	u16 txcf;		/* Total number of PAUSE control frames
				   transmitted by this MAC */
	u32 tmca;		/* Total number of frames that were transmitted
				   successfully with the group address bit set
				   that are not broadcast frames */
	u32 tbca;		/* Total number of frames transmitted
				   successfully that had destination address
				   field equal to the broadcast address */
	u32 rxfok;		/* Total number of frames received OK */
	u32 rxbok;		/* Total number of octets received OK */
	u32 rbyt;		/* Total number of octets received including
				   octets in bad frames. Must be implemented in
				   HW because it includes octets in frames that
				   never even reach the UCC */
	u32 rmca;		/* Total number of frames that were received
				   successfully with the group address bit set
				   that are not broadcast frames */
	u32 rbca;		/* Total number of frames received successfully
				   that had destination address equal to the
				   broadcast address */
} __packed;

/* UCC GETH Tx errors returned via TxConf callback */
#define TX_ERRORS_DEF      0x0200
#define TX_ERRORS_EXDEF    0x0100
#define TX_ERRORS_LC       0x0080
#define TX_ERRORS_RL       0x0040
#define TX_ERRORS_RC_MASK  0x003C
#define TX_ERRORS_RC_SHIFT 2
#define TX_ERRORS_UN       0x0002
#define TX_ERRORS_CSL      0x0001

/* UCC GETH Rx errors returned via RxStore callback */
#define RX_ERRORS_CMR      0x0200
#define RX_ERRORS_M        0x0100
#define RX_ERRORS_BC       0x0080
#define RX_ERRORS_MC       0x0040

/* Transmit BD. These are in addition to values defined in uccf. */
#define T_VID      0x003c0000	/* insert VLAN id index mask. */
#define T_DEF      (((u32) TX_ERRORS_DEF     ) << 16)
#define T_EXDEF    (((u32) TX_ERRORS_EXDEF   ) << 16)
#define T_LC       (((u32) TX_ERRORS_LC      ) << 16)
#define T_RL       (((u32) TX_ERRORS_RL      ) << 16)
#define T_RC_MASK  (((u32) TX_ERRORS_RC_MASK ) << 16)
#define T_UN       (((u32) TX_ERRORS_UN      ) << 16)
#define T_CSL      (((u32) TX_ERRORS_CSL     ) << 16)
#define T_ERRORS_REPORT  (T_DEF | T_EXDEF | T_LC | T_RL | T_RC_MASK \
		| T_UN | T_CSL)	/* transmit errors to report */

/* Receive BD. These are in addition to values defined in uccf. */
#define R_LG    0x00200000	/* Frame length violation.  */
#define R_NO    0x00100000	/* Non-octet aligned frame.  */
#define R_SH    0x00080000	/* Short frame.  */
#define R_CR    0x00040000	/* CRC error.  */
#define R_OV    0x00020000	/* Overrun.  */
#define R_IPCH  0x00010000	/* IP checksum check failed. */
#define R_CMR   (((u32) RX_ERRORS_CMR  ) << 16)
#define R_M     (((u32) RX_ERRORS_M    ) << 16)
#define R_BC    (((u32) RX_ERRORS_BC   ) << 16)
#define R_MC    (((u32) RX_ERRORS_MC   ) << 16)
#define R_ERRORS_REPORT (R_CMR | R_M | R_BC | R_MC)	/* receive errors to
							   report */
#define R_ERRORS_FATAL  (R_LG  | R_NO | R_SH | R_CR | \
		R_OV | R_IPCH)	/* receive errors to discard */

/* Alignments */
#define UCC_GETH_RX_GLOBAL_PRAM_ALIGNMENT	256
#define UCC_GETH_TX_GLOBAL_PRAM_ALIGNMENT       128
#define UCC_GETH_THREAD_RX_PRAM_ALIGNMENT       128
#define UCC_GETH_THREAD_TX_PRAM_ALIGNMENT       64
#define UCC_GETH_THREAD_DATA_ALIGNMENT          256	/* spec gives values
							   based on num of
							   threads, but always
							   using the maximum is
							   easier */
#define UCC_GETH_SEND_QUEUE_QUEUE_DESCRIPTOR_ALIGNMENT	32
#define UCC_GETH_SCHEDULER_ALIGNMENT		8	/* This is a guess */
#define UCC_GETH_TX_STATISTICS_ALIGNMENT	4	/* This is a guess */
#define UCC_GETH_RX_STATISTICS_ALIGNMENT	4	/* This is a guess */
#define UCC_GETH_RX_INTERRUPT_COALESCING_ALIGNMENT	64
#define UCC_GETH_RX_BD_QUEUES_ALIGNMENT		8	/* This is a guess */
#define UCC_GETH_RX_PREFETCHED_BDS_ALIGNMENT	128	/* This is a guess */
#define UCC_GETH_RX_EXTENDED_FILTERING_GLOBAL_PARAMETERS_ALIGNMENT 8	/* This
									   is a
									   guess
									 */
#define UCC_GETH_RX_BD_RING_ALIGNMENT		32
#define UCC_GETH_TX_BD_RING_ALIGNMENT		32
#define UCC_GETH_MRBLR_ALIGNMENT		128
#define UCC_GETH_RX_BD_RING_SIZE_ALIGNMENT	4
#define UCC_GETH_TX_BD_RING_SIZE_MEMORY_ALIGNMENT	32
#define UCC_GETH_RX_DATA_BUF_ALIGNMENT		64

#define UCC_GETH_TAD_EF                         0x80
#define UCC_GETH_TAD_V                          0x40
#define UCC_GETH_TAD_REJ                        0x20
#define UCC_GETH_TAD_VTAG_OP_RIGHT_SHIFT        2
#define UCC_GETH_TAD_VTAG_OP_SHIFT              6
#define UCC_GETH_TAD_V_NON_VTAG_OP              0x20
#define UCC_GETH_TAD_RQOS_SHIFT                 0
#define UCC_GETH_TAD_V_PRIORITY_SHIFT           5
#define UCC_GETH_TAD_CFI                        0x10

#define UCC_GETH_VLAN_PRIORITY_MAX              8
#define UCC_GETH_IP_PRIORITY_MAX                64
#define UCC_GETH_TX_VTAG_TABLE_ENTRY_MAX        8
#define UCC_GETH_RX_BD_RING_SIZE_MIN            8
#define UCC_GETH_TX_BD_RING_SIZE_MIN            2
#define UCC_GETH_BD_RING_SIZE_MAX		0xffff

#define UCC_GETH_SIZE_OF_BD                     QE_SIZEOF_BD

/* Driver definitions */
#define TX_BD_RING_LEN                          0x10
#define RX_BD_RING_LEN                          0x10

#define TX_RING_MOD_MASK(size)                  (size-1)
#define RX_RING_MOD_MASK(size)                  (size-1)

#define ENET_NUM_OCTETS_PER_ADDRESS             6
#define ENET_GROUP_ADDR                         0x01	/* Group address mask
							   for ethernet
							   addresses */

#define TX_TIMEOUT                              (1*HZ)
#define SKB_ALLOC_TIMEOUT                       100000
#define PHY_INIT_TIMEOUT                        100000
#define PHY_CHANGE_TIME                         2

/* Fast Ethernet (10/100 Mbps) */
#define UCC_GETH_URFS_INIT                      512	/* Rx virtual FIFO size
							 */
#define UCC_GETH_URFET_INIT                     256	/* 1/2 urfs */
#define UCC_GETH_URFSET_INIT                    384	/* 3/4 urfs */
#define UCC_GETH_UTFS_INIT                      512	/* Tx virtual FIFO size
							 */
#define UCC_GETH_UTFET_INIT                     256	/* 1/2 utfs */
#define UCC_GETH_UTFTT_INIT                     512
/* Gigabit Ethernet (1000 Mbps) */
#define UCC_GETH_URFS_GIGA_INIT                 4096/*2048*/	/* Rx virtual
								   FIFO size */
#define UCC_GETH_URFET_GIGA_INIT                2048/*1024*/	/* 1/2 urfs */
#define UCC_GETH_URFSET_GIGA_INIT               3072/*1536*/	/* 3/4 urfs */
#define UCC_GETH_UTFS_GIGA_INIT                 4096/*2048*/	/* Tx virtual
								   FIFO size */
#define UCC_GETH_UTFET_GIGA_INIT                2048/*1024*/	/* 1/2 utfs */
#define UCC_GETH_UTFTT_GIGA_INIT                4096/*0x40*/	/* Tx virtual
								   FIFO size */

#define UCC_GETH_REMODER_INIT                   0	/* bits that must be
							   set */
#define UCC_GETH_TEMODER_INIT                   0xC000	/* bits that must */

/* Initial value for UPSMR */
#define UCC_GETH_UPSMR_INIT                     UCC_GETH_UPSMR_RES1

#define UCC_GETH_MACCFG1_INIT                   0
#define UCC_GETH_MACCFG2_INIT                   (MACCFG2_RESERVED_1)

/* Ethernet Address Type. */
enum enet_addr_type {
	ENET_ADDR_TYPE_INDIVIDUAL,
	ENET_ADDR_TYPE_GROUP,
	ENET_ADDR_TYPE_BROADCAST
};

/* UCC GETH 82xx Ethernet Address Recognition Location */
enum ucc_geth_enet_address_recognition_location {
	UCC_GETH_ENET_ADDRESS_RECOGNITION_LOCATION_STATION_ADDRESS,/* station
								      address */
	UCC_GETH_ENET_ADDRESS_RECOGNITION_LOCATION_PADDR_FIRST,	/* additional
								   station
								   address
								   paddr1 */
	UCC_GETH_ENET_ADDRESS_RECOGNITION_LOCATION_PADDR2,	/* additional
								   station
								   address
								   paddr2 */
	UCC_GETH_ENET_ADDRESS_RECOGNITION_LOCATION_PADDR3,	/* additional
								   station
								   address
								   paddr3 */
	UCC_GETH_ENET_ADDRESS_RECOGNITION_LOCATION_PADDR_LAST,	/* additional
								   station
								   address
								   paddr4 */
	UCC_GETH_ENET_ADDRESS_RECOGNITION_LOCATION_GROUP_HASH,	/* group hash */
	UCC_GETH_ENET_ADDRESS_RECOGNITION_LOCATION_INDIVIDUAL_HASH /* individual
								      hash */
};

/* UCC GETH vlan operation tagged */
enum ucc_geth_vlan_operation_tagged {
	UCC_GETH_VLAN_OPERATION_TAGGED_NOP = 0x0,	/* Tagged - nop */
	UCC_GETH_VLAN_OPERATION_TAGGED_REPLACE_VID_PORTION_OF_Q_TAG
		= 0x1,	/* Tagged - replace vid portion of q tag */
	UCC_GETH_VLAN_OPERATION_TAGGED_IF_VID0_REPLACE_VID_WITH_DEFAULT_VALUE
		= 0x2,	/* Tagged - if vid0 replace vid with default value  */
	UCC_GETH_VLAN_OPERATION_TAGGED_EXTRACT_Q_TAG_FROM_FRAME
		= 0x3	/* Tagged - extract q tag from frame */
};

/* UCC GETH vlan operation non-tagged */
enum ucc_geth_vlan_operation_non_tagged {
	UCC_GETH_VLAN_OPERATION_NON_TAGGED_NOP = 0x0,	/* Non tagged - nop */
	UCC_GETH_VLAN_OPERATION_NON_TAGGED_Q_TAG_INSERT = 0x1	/* Non tagged -
								   q tag insert
								 */
};

/* UCC GETH Rx Quality of Service Mode */
enum ucc_geth_qos_mode {
	UCC_GETH_QOS_MODE_DEFAULT = 0x0,	/* default queue */
	UCC_GETH_QOS_MODE_QUEUE_NUM_FROM_L2_CRITERIA = 0x1,	/* queue
								   determined
								   by L2
								   criteria */
	UCC_GETH_QOS_MODE_QUEUE_NUM_FROM_L3_CRITERIA = 0x2	/* queue
								   determined
								   by L3
								   criteria */
};

/* UCC GETH Statistics Gathering Mode - These are bit flags, 'or' them together
   for combined functionality */
enum ucc_geth_statistics_gathering_mode {
	UCC_GETH_STATISTICS_GATHERING_MODE_NONE = 0x00000000,	/* No
								   statistics
								   gathering */
	UCC_GETH_STATISTICS_GATHERING_MODE_HARDWARE = 0x00000001,/* Enable
								    hardware
								    statistics
								    gathering
								  */
	UCC_GETH_STATISTICS_GATHERING_MODE_FIRMWARE_TX = 0x00000004,/*Enable
								      firmware
								      tx
								      statistics
								      gathering
								     */
	UCC_GETH_STATISTICS_GATHERING_MODE_FIRMWARE_RX = 0x00000008/* Enable
								      firmware
								      rx
								      statistics
								      gathering
								    */
};

/* UCC GETH Pad and CRC Mode - Note, Padding without CRC is not possible */
enum ucc_geth_maccfg2_pad_and_crc_mode {
	UCC_GETH_PAD_AND_CRC_MODE_NONE
		= MACCFG2_PAD_AND_CRC_MODE_NONE,	/* Neither Padding
							   short frames
							   nor CRC */
	UCC_GETH_PAD_AND_CRC_MODE_CRC_ONLY
		= MACCFG2_PAD_AND_CRC_MODE_CRC_ONLY,	/* Append
							   CRC only */
	UCC_GETH_PAD_AND_CRC_MODE_PAD_AND_CRC =
	    MACCFG2_PAD_AND_CRC_MODE_PAD_AND_CRC
};

/* UCC GETH upsmr Flow Control Mode */
enum ucc_geth_flow_control_mode {
	UPSMR_AUTOMATIC_FLOW_CONTROL_MODE_NONE = 0x00000000,	/* No automatic
								   flow control
								 */
	UPSMR_AUTOMATIC_FLOW_CONTROL_MODE_PAUSE_WHEN_EMERGENCY
		= 0x00004000	/* Send pause frame when RxFIFO reaches its
				   emergency threshold */
};

/* UCC GETH number of threads */
enum ucc_geth_num_of_threads {
	UCC_GETH_NUM_OF_THREADS_1 = 0x1,	/* 1 */
	UCC_GETH_NUM_OF_THREADS_2 = 0x2,	/* 2 */
	UCC_GETH_NUM_OF_THREADS_4 = 0x0,	/* 4 */
	UCC_GETH_NUM_OF_THREADS_6 = 0x3,	/* 6 */
	UCC_GETH_NUM_OF_THREADS_8 = 0x4	/* 8 */
};

/* UCC GETH number of station addresses */
enum ucc_geth_num_of_station_addresses {
	UCC_GETH_NUM_OF_STATION_ADDRESSES_1,	/* 1 */
	UCC_GETH_NUM_OF_STATION_ADDRESSES_5	/* 5 */
};

/* UCC GETH 82xx Ethernet Address Container */
struct enet_addr_container {
	u8 address[ENET_NUM_OCTETS_PER_ADDRESS];	/* ethernet address */
	enum ucc_geth_enet_address_recognition_location location;	/* location in
								   82xx address
								   recognition
								   hardware */
	struct list_head node;
};

#define ENET_ADDR_CONT_ENTRY(ptr) list_entry(ptr, struct enet_addr_container, node)

/* UCC GETH Termination Action Descriptor (TAD) structure. */
struct ucc_geth_tad_params {
	int rx_non_dynamic_extended_features_mode;
	int reject_frame;
	enum ucc_geth_vlan_operation_tagged vtag_op;
	enum ucc_geth_vlan_operation_non_tagged vnontag_op;
	enum ucc_geth_qos_mode rqos;
	u8 vpri;
	u16 vid;
};

/* GETH protocol initialization structure */
struct ucc_geth_info {
	struct ucc_fast_info uf_info;
	u8 numQueuesTx;
	u8 numQueuesRx;
	int ipCheckSumCheck;
	int ipCheckSumGenerate;
	int rxExtendedFiltering;
	u32 extendedFilteringChainPointer;
	u16 typeorlen;
	int dynamicMaxFrameLength;
	int dynamicMinFrameLength;
	u8 nonBackToBackIfgPart1;
	u8 nonBackToBackIfgPart2;
	u8 miminumInterFrameGapEnforcement;
	u8 backToBackInterFrameGap;
	int ipAddressAlignment;
	int lengthCheckRx;
	u32 mblinterval;
	u16 nortsrbytetime;
	u8 fracsiz;
	u8 strictpriorityq;
	u8 txasap;
	u8 extrabw;
	int miiPreambleSupress;
	u8 altBebTruncation;
	int altBeb;
	int backPressureNoBackoff;
	int noBackoff;
	int excessDefer;
	u8 maxRetransmission;
	u8 collisionWindow;
	int pro;
	int cap;
	int rsh;
	int rlpb;
	int cam;
	int bro;
	int ecm;
	int receiveFlowControl;
	int transmitFlowControl;
	u8 maxGroupAddrInHash;
	u8 maxIndAddrInHash;
	u8 prel;
	u16 maxFrameLength;
	u16 minFrameLength;
	u16 maxD1Length;
	u16 maxD2Length;
	u16 vlantype;
	u16 vlantci;
	u32 ecamptr;
	u32 eventRegMask;
	u16 pausePeriod;
	u16 extensionField;
	struct device_node *phy_node;
	struct device_node *tbi_node;
	u8 weightfactor[NUM_TX_QUEUES];
	u8 interruptcoalescingmaxvalue[NUM_RX_QUEUES];
	u8 l2qt[UCC_GETH_VLAN_PRIORITY_MAX];
	u8 l3qt[UCC_GETH_IP_PRIORITY_MAX];
	u32 vtagtable[UCC_GETH_TX_VTAG_TABLE_ENTRY_MAX];
	u8 iphoffset[TX_IP_OFFSET_ENTRY_MAX];
	u16 bdRingLenTx[NUM_TX_QUEUES];
	u16 bdRingLenRx[NUM_RX_QUEUES];
	enum ucc_geth_num_of_station_addresses numStationAddresses;
	enum qe_fltr_largest_external_tbl_lookup_key_size
	    largestexternallookupkeysize;
	enum ucc_geth_statistics_gathering_mode statisticsMode;
	enum ucc_geth_vlan_operation_tagged vlanOperationTagged;
	enum ucc_geth_vlan_operation_non_tagged vlanOperationNonTagged;
	enum ucc_geth_qos_mode rxQoSMode;
	enum ucc_geth_flow_control_mode aufc;
	enum ucc_geth_maccfg2_pad_and_crc_mode padAndCrc;
	enum ucc_geth_num_of_threads numThreadsTx;
	enum ucc_geth_num_of_threads numThreadsRx;
	unsigned int riscTx;
	unsigned int riscRx;
};

/* structure representing UCC GETH */
struct ucc_geth_private {
	struct ucc_geth_info *ug_info;
	struct ucc_fast_private *uccf;
	struct device *dev;
	struct net_device *ndev;
	struct napi_struct napi;
	struct work_struct timeout_work;
	struct ucc_geth __iomem *ug_regs;
	struct ucc_geth_init_pram *p_init_enet_param_shadow;
	struct ucc_geth_exf_global_pram __iomem *p_exf_glbl_param;
	u32 exf_glbl_param_offset;
	struct ucc_geth_rx_global_pram __iomem *p_rx_glbl_pram;
	u32 rx_glbl_pram_offset;
	struct ucc_geth_tx_global_pram __iomem *p_tx_glbl_pram;
	u32 tx_glbl_pram_offset;
	struct ucc_geth_send_queue_mem_region __iomem *p_send_q_mem_reg;
	u32 send_q_mem_reg_offset;
	struct ucc_geth_thread_data_tx __iomem *p_thread_data_tx;
	u32 thread_dat_tx_offset;
	struct ucc_geth_thread_data_rx __iomem *p_thread_data_rx;
	u32 thread_dat_rx_offset;
	struct ucc_geth_scheduler __iomem *p_scheduler;
	u32 scheduler_offset;
	struct ucc_geth_tx_firmware_statistics_pram __iomem *p_tx_fw_statistics_pram;
	u32 tx_fw_statistics_pram_offset;
	struct ucc_geth_rx_firmware_statistics_pram __iomem *p_rx_fw_statistics_pram;
	u32 rx_fw_statistics_pram_offset;
	struct ucc_geth_rx_interrupt_coalescing_table __iomem *p_rx_irq_coalescing_tbl;
	u32 rx_irq_coalescing_tbl_offset;
	struct ucc_geth_rx_bd_queues_entry __iomem *p_rx_bd_qs_tbl;
	u32 rx_bd_qs_tbl_offset;
	u8 __iomem *p_tx_bd_ring[NUM_TX_QUEUES];
	u32 tx_bd_ring_offset[NUM_TX_QUEUES];
	u8 __iomem *p_rx_bd_ring[NUM_RX_QUEUES];
	u32 rx_bd_ring_offset[NUM_RX_QUEUES];
	u8 __iomem *confBd[NUM_TX_QUEUES];
	u8 __iomem *txBd[NUM_TX_QUEUES];
	u8 __iomem *rxBd[NUM_RX_QUEUES];
	int badFrame[NUM_RX_QUEUES];
	u16 cpucount[NUM_TX_QUEUES];
	u16 __iomem *p_cpucount[NUM_TX_QUEUES];
	int indAddrRegUsed[NUM_OF_PADDRS];
	u8 paddr[NUM_OF_PADDRS][ENET_NUM_OCTETS_PER_ADDRESS];	/* ethernet address */
	u8 numGroupAddrInHash;
	u8 numIndAddrInHash;
	u8 numIndAddrInReg;
	int rx_extended_features;
	int rx_non_dynamic_extended_features;
	struct list_head conf_skbs;
	struct list_head group_hash_q;
	struct list_head ind_hash_q;
	u32 saved_uccm;
	spinlock_t lock;
	/* pointers to arrays of skbuffs for tx and rx */
	struct sk_buff **tx_skbuff[NUM_TX_QUEUES];
	struct sk_buff **rx_skbuff[NUM_RX_QUEUES];
	/* indices pointing to the next free sbk in skb arrays */
	u16 skb_curtx[NUM_TX_QUEUES];
	u16 skb_currx[NUM_RX_QUEUES];
	/* index of the first skb which hasn't been transmitted yet. */
	u16 skb_dirtytx[NUM_TX_QUEUES];

	struct sk_buff_head rx_recycle;

	struct ugeth_mii_info *mii_info;
	struct phy_device *phydev;
	phy_interface_t phy_interface;
	int max_speed;
	uint32_t msg_enable;
	int oldspeed;
	int oldduplex;
	int oldlink;
	int wol_en;

	struct device_node *node;
};

void uec_set_ethtool_ops(struct net_device *netdev);
int init_flow_control_params(u32 automatic_flow_control_mode,
		int rx_flow_control_enable, int tx_flow_control_enable,
		u16 pause_period, u16 extension_field,
		u32 __iomem *upsmr_register, u32 __iomem *uempr_register,
		u32 __iomem *maccfg1_register);


#endif				/* __UCC_GETH_H__ */