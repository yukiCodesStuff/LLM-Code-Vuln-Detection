#define B_AX_CH10_BUSY			BIT(0)

/* Configure */
#define R_AX_PCIE_INIT_CFG2		0x1004
#define B_AX_WD_ITVL_IDLE		GENMASK(27, 24)
#define B_AX_WD_ITVL_ACT		GENMASK(19, 16)
