#define CSR_GPIO_IN             (CSR_BASE+0x018) /* read external chip pins */
#define CSR_RESET               (CSR_BASE+0x020) /* busmaster enable, NMI, etc*/
#define CSR_GP_CNTRL            (CSR_BASE+0x024)
#define CSR_FUNC_SCRATCH        (CSR_BASE+0x02c) /* Scratch register - used for FW dbg */

/* 2nd byte of CSR_INT_COALESCING, not accessible via iwl_write32()! */
#define CSR_INT_PERIODIC_REG	(CSR_BASE+0x005)

#define CSR_DBG_HPET_MEM_REG		(CSR_BASE+0x240)
#define CSR_DBG_LINK_PWR_MGMT_REG	(CSR_BASE+0x250)

/*
 * Scratch register initial configuration - this is set on init, and read
 * during a error FW error.
 */
#define CSR_FUNC_SCRATCH_INIT_VALUE		(0x01010101)

/* Bits for CSR_HW_IF_CONFIG_REG */
#define CSR_HW_IF_CONFIG_REG_MSK_MAC_DASH	(0x00000003)
#define CSR_HW_IF_CONFIG_REG_MSK_MAC_STEP	(0x0000000C)
#define CSR_HW_IF_CONFIG_REG_BIT_MONITOR_SRAM	(0x00000080)
	MSIX_HW_INT_CAUSES_REG_WAKEUP		= BIT(1),
	MSIX_HW_INT_CAUSES_REG_IML              = BIT(1),
	MSIX_HW_INT_CAUSES_REG_RESET_DONE	= BIT(2),
	MSIX_HW_INT_CAUSES_REG_SW_ERR_BZ	= BIT(5),
	MSIX_HW_INT_CAUSES_REG_CT_KILL		= BIT(6),
	MSIX_HW_INT_CAUSES_REG_RF_KILL		= BIT(7),
	MSIX_HW_INT_CAUSES_REG_PERIODIC		= BIT(8),
	MSIX_HW_INT_CAUSES_REG_SW_ERR		= BIT(25),