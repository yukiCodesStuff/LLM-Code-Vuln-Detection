	union {
		struct omap_hwmod_omap2_prcm omap2;
		struct omap_hwmod_omap4_prcm omap4;
	}				prcm;
	const char			*main_clk;
	struct clk			*_clk;
	struct omap_hwmod_opt_clk	*opt_clks;
	char				*clkdm_name;
	struct clockdomain		*clkdm;
	struct list_head		master_ports; /* connect to *_IA */
	struct list_head		slave_ports; /* connect to *_TA */
	void				*dev_attr;
	u32				_sysc_cache;
	void __iomem			*_mpu_rt_va;
	spinlock_t			_lock;
	struct list_head		node;
	struct omap_hwmod_ocp_if	*_mpu_port;
	u16				flags;
	u8				mpu_rt_idx;
	u8				response_lat;
	u8				rst_lines_cnt;
	u8				opt_clks_cnt;
	u8				masters_cnt;
	u8				slaves_cnt;
	u8				hwmods_cnt;
	u8				_int_flags;
	u8				_state;
	u8				_postsetup_state;
	struct omap_hwmod		*parent_hwmod;
};

struct omap_hwmod *omap_hwmod_lookup(const char *name);
int omap_hwmod_for_each(int (*fn)(struct omap_hwmod *oh, void *data),
			void *data);

int __init omap_hwmod_setup_one(const char *name);

int omap_hwmod_enable(struct omap_hwmod *oh);
int omap_hwmod_idle(struct omap_hwmod *oh);
int omap_hwmod_shutdown(struct omap_hwmod *oh);

int omap_hwmod_assert_hardreset(struct omap_hwmod *oh, const char *name);
int omap_hwmod_deassert_hardreset(struct omap_hwmod *oh, const char *name);
int omap_hwmod_read_hardreset(struct omap_hwmod *oh, const char *name);

int omap_hwmod_enable_clocks(struct omap_hwmod *oh);
int omap_hwmod_disable_clocks(struct omap_hwmod *oh);

int omap_hwmod_reset(struct omap_hwmod *oh);
void omap_hwmod_ocp_barrier(struct omap_hwmod *oh);

void omap_hwmod_write(u32 v, struct omap_hwmod *oh, u16 reg_offs);
u32 omap_hwmod_read(struct omap_hwmod *oh, u16 reg_offs);
int omap_hwmod_softreset(struct omap_hwmod *oh);

int omap_hwmod_count_resources(struct omap_hwmod *oh, unsigned long flags);
int omap_hwmod_fill_resources(struct omap_hwmod *oh, struct resource *res);
int omap_hwmod_fill_dma_resources(struct omap_hwmod *oh, struct resource *res);
int omap_hwmod_get_resource_byname(struct omap_hwmod *oh, unsigned int type,
				   const char *name, struct resource *res);

struct powerdomain *omap_hwmod_get_pwrdm(struct omap_hwmod *oh);
void __iomem *omap_hwmod_get_mpu_rt_va(struct omap_hwmod *oh);

int omap_hwmod_add_initiator_dep(struct omap_hwmod *oh,
				 struct omap_hwmod *init_oh);
int omap_hwmod_del_initiator_dep(struct omap_hwmod *oh,
				 struct omap_hwmod *init_oh);

int omap_hwmod_enable_wakeup(struct omap_hwmod *oh);
int omap_hwmod_disable_wakeup(struct omap_hwmod *oh);

int omap_hwmod_for_each_by_class(const char *classname,
				 int (*fn)(struct omap_hwmod *oh,
					   void *user),
				 void *user);

int omap_hwmod_set_postsetup_state(struct omap_hwmod *oh, u8 state);
int omap_hwmod_get_context_loss_count(struct omap_hwmod *oh);

int omap_hwmod_no_setup_reset(struct omap_hwmod *oh);

int omap_hwmod_pad_route_irq(struct omap_hwmod *oh, int pad_idx, int irq_idx);

extern void __init omap_hwmod_init(void);

const char *omap_hwmod_get_main_clk(struct omap_hwmod *oh);

/*
 *
 */

extern int omap_hwmod_aess_preprogram(struct omap_hwmod *oh);

/*
 * Chip variant-specific hwmod init routines - XXX should be converted
 * to use initcalls once the initial boot ordering is straightened out
 */
extern int omap2420_hwmod_init(void);
extern int omap2430_hwmod_init(void);
extern int omap3xxx_hwmod_init(void);
extern int omap44xx_hwmod_init(void);
extern int omap54xx_hwmod_init(void);
extern int am33xx_hwmod_init(void);
extern int dra7xx_hwmod_init(void);
int am43xx_hwmod_init(void);

extern int __init omap_hwmod_register_links(struct omap_hwmod_ocp_if **ois);

#endif