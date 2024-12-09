typedef enum {
	RX0_ID = 0,
	N_RX_ID
} rx_ID_t;

typedef enum {
	MIPI_PORT0_ID = 0,
	MIPI_PORT1_ID,
	MIPI_PORT2_ID,
	N_MIPI_PORT_ID
} mipi_port_ID_t;

#define	N_RX_CHANNEL_ID		4

/* Generic port enumeration with an internal port type ID */
typedef enum {
	CSI_PORT0_ID = 0,
	CSI_PORT1_ID,
	CSI_PORT2_ID,
	TPG_PORT0_ID,
	PRBS_PORT0_ID,
	FIFO_PORT0_ID,
	MEMORY_PORT0_ID,
	N_INPUT_PORT_ID
} input_port_ID_t;

typedef enum {
	CAPTURE_UNIT0_ID = 0,
	CAPTURE_UNIT1_ID,
	CAPTURE_UNIT2_ID,
	ACQUISITION_UNIT0_ID,
	DMA_UNIT0_ID,
	CTRL_UNIT0_ID,
	GPREGS_UNIT0_ID,
	FIFO_UNIT0_ID,
	IRQ_UNIT0_ID,
	N_SUB_SYSTEM_ID
} sub_system_ID_t;

#define	N_CAPTURE_UNIT_ID		3
#define	N_ACQUISITION_UNIT_ID	1
#define	N_CTRL_UNIT_ID			1

enum ia_css_isp_memories {
	IA_CSS_ISP_PMEM0 = 0,
	IA_CSS_ISP_DMEM0,
	IA_CSS_ISP_VMEM0,
	IA_CSS_ISP_VAMEM0,
	IA_CSS_ISP_VAMEM1,
	IA_CSS_ISP_VAMEM2,
	IA_CSS_ISP_HMEM0,
	IA_CSS_SP_DMEM0,
	IA_CSS_DDR,
	N_IA_CSS_MEMORIES
};
#define IA_CSS_NUM_MEMORIES 9
/* For driver compatability */
#define N_IA_CSS_ISP_MEMORIES   IA_CSS_NUM_MEMORIES
#define IA_CSS_NUM_ISP_MEMORIES IA_CSS_NUM_MEMORIES

#if 0
typedef enum {
	dev_chn, /* device channels, external resource */
	ext_mem, /* external memories */
	int_mem, /* internal memories */
	int_chn  /* internal channels, user defined */
} resource_type_t;

/* if this enum is extended with other memory resources, pls also extend the function resource_to_memptr() */
typedef enum {
	vied_nci_dev_chn_dma_ext0,
	int_mem_vmem0,
	int_mem_dmem0
} resource_id_t;

/* enum listing the different memories within a program group.
   This enum is used in the mem_ptr_t type */
typedef enum {
	buf_mem_invalid = 0,
	buf_mem_vmem_prog0,
	buf_mem_dmem_prog0
} buf_mem_t;

#endif
#endif /* __SYSTEM_GLOBAL_H_INCLUDED__ */