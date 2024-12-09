#define RADIO_REG_SYS_MANUAL_DFT_0	0xAD4078
#define RFIC_REG_RD			0xAD0470
#define WFPM_CTRL_REG			0xA03030
#define WFPM_CTRL_REG_GEN2		0xd03030
#define WFPM_OTP_CFG1_ADDR		0x00a03098
#define WFPM_OTP_CFG1_ADDR_GEN2		0x00d03098
#define WFPM_OTP_CFG1_IS_JACKET_BIT	BIT(4)
#define WFPM_OTP_CFG1_IS_CDB_BIT	BIT(5)

#define WFPM_GP2			0xA030B4

/* DBGI SRAM Register details */
#define DBGI_SRAM_TARGET_ACCESS_CFG			0x00A2E14C
	LMPM_PAGE_PASS_NOTIF_POS = BIT(20),
};

/*
 * CRF ID register
 *
 * type: bits 0-11
 * reserved: bits 12-18
 * slave_exist: bit 19
 * dash: bits 20-23
 * step: bits 24-26
 * flavor: bits 27-31
 */
#define REG_CRF_ID_TYPE(val)		(((val) & 0x00000FFF) >> 0)
#define REG_CRF_ID_SLAVE(val)		(((val) & 0x00080000) >> 19)
#define REG_CRF_ID_DASH(val)		(((val) & 0x00F00000) >> 20)
#define REG_CRF_ID_STEP(val)		(((val) & 0x07000000) >> 24)
#define REG_CRF_ID_FLAVOR(val)		(((val) & 0xF8000000) >> 27)

#define UREG_CHICK		(0xA05C00)
#define UREG_CHICK_MSI_ENABLE	BIT(24)
#define UREG_CHICK_MSIX_ENABLE	BIT(25)

#define SD_REG_VER		0xa29600
#define SD_REG_VER_GEN2		0x00a2b800

#define REG_CRF_ID_TYPE_JF_1			0x201
#define REG_CRF_ID_TYPE_JF_2			0x202
#define REG_CRF_ID_TYPE_HR_CDB			0x503
#define REG_CRF_ID_TYPE_HR_NONE_CDB		0x504
#define REG_CRF_ID_TYPE_HR_NONE_CDB_1X1	0x501
#define REG_CRF_ID_TYPE_HR_NONE_CDB_CCP	0x532
#define REG_CRF_ID_TYPE_GF			0x410
#define REG_CRF_ID_TYPE_GF_TC			0xF08
#define REG_CRF_ID_TYPE_MR			0x810
#define REG_CRF_ID_TYPE_FM			0x910

#define HPM_DEBUG			0xA03440
#define PERSISTENCE_BIT			BIT(12)
#define PREG_WFPM_ACCESS		BIT(12)
