#define MT_HIF_REMAP_L2_BASE		GENMASK(31, 12)
#define MT_HIF_REMAP_BASE_L2		0x00000

#define MT_SWDEF_BASE			0x41f200
#define MT_SWDEF(ofs)			(MT_SWDEF_BASE + (ofs))
#define MT_SWDEF_MODE			MT_SWDEF(0x3c)
#define MT_SWDEF_NORMAL_MODE		0
#define MT_WF_PHY_RXTD12_IRPI_SW_CLR_ONLY	BIT(18)
#define MT_WF_PHY_RXTD12_IRPI_SW_CLR	BIT(29)

#endif