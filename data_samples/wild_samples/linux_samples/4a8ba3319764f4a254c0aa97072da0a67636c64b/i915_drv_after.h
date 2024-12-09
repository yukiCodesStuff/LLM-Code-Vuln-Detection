		union {
			struct ilk_wm_values hw;
			struct skl_wm_values skl_hw;
		};
	} wm;

	struct i915_runtime_pm pm;

	struct intel_digital_port *hpd_irq_port[I915_MAX_PORTS];
	u32 long_hpd_port_mask;
	u32 short_hpd_port_mask;
	struct work_struct dig_port_work;

	/*
	 * if we get a HPD irq from DP and a HPD irq from non-DP
	 * the non-DP HPD could block the workqueue on a mode config
	 * mutex getting, that userspace may have taken. However
	 * userspace is waiting on the DP workqueue to run which is
	 * blocked behind the non-DP one.
	 */
	struct workqueue_struct *dp_wq;

	/* Abstract the submission mechanism (legacy ringbuffer or execlists) away */
	struct {
		int (*do_execbuf)(struct drm_device *dev, struct drm_file *file,
				  struct intel_engine_cs *ring,
				  struct intel_context *ctx,
				  struct drm_i915_gem_execbuffer2 *args,
				  struct list_head *vmas,
				  struct drm_i915_gem_object *batch_obj,
				  u64 exec_start, u32 flags);
		int (*init_rings)(struct drm_device *dev);
		void (*cleanup_ring)(struct intel_engine_cs *ring);
		void (*stop_ring)(struct intel_engine_cs *ring);
	} gt;

	/*
	 * NOTE: This is the dri1/ums dungeon, don't add stuff here. Your patch
	 * will be rejected. Instead look for a better place.
	 */
};

static inline struct drm_i915_private *to_i915(const struct drm_device *dev)
{
	return dev->dev_private;
}

/* Iterate over initialised rings */
#define for_each_ring(ring__, dev_priv__, i__) \
	for ((i__) = 0; (i__) < I915_NUM_RINGS; (i__)++) \
		if (((ring__) = &(dev_priv__)->ring[(i__)]), intel_ring_initialized((ring__)))

enum hdmi_force_audio {
	HDMI_AUDIO_OFF_DVI = -2,	/* no aux data for HDMI-DVI converter */
	HDMI_AUDIO_OFF,			/* force turn off HDMI audio */
	HDMI_AUDIO_AUTO,		/* trust EDID */
	HDMI_AUDIO_ON,			/* force turn on HDMI audio */
};

#define I915_GTT_OFFSET_NONE ((u32)-1)

struct drm_i915_gem_object_ops {
	/* Interface between the GEM object and its backing storage.
	 * get_pages() is called once prior to the use of the associated set
	 * of pages before to binding them into the GTT, and put_pages() is
	 * called after we no longer need them. As we expect there to be
	 * associated cost with migrating pages between the backing storage
	 * and making them available for the GPU (e.g. clflush), we may hold
	 * onto the pages after they are no longer referenced by the GPU
	 * in case they may be used again shortly (for example migrating the
	 * pages to a different memory domain within the GTT). put_pages()
	 * will therefore most likely be called when the object itself is
	 * being released or under memory pressure (where we attempt to
	 * reap pages for the shrinker).
	 */
	int (*get_pages)(struct drm_i915_gem_object *);
	void (*put_pages)(struct drm_i915_gem_object *);
	int (*dmabuf_export)(struct drm_i915_gem_object *);
	void (*release)(struct drm_i915_gem_object *);
};

/*
 * Frontbuffer tracking bits. Set in obj->frontbuffer_bits while a gem bo is
 * considered to be the frontbuffer for the given plane interface-vise. This
 * doesn't mean that the hw necessarily already scans it out, but that any
 * rendering (by the cpu or gpu) will land in the frontbuffer eventually.
 *
 * We have one bit per pipe and per scanout plane type.
 */
#define INTEL_FRONTBUFFER_BITS_PER_PIPE 4
#define INTEL_FRONTBUFFER_BITS \
	(INTEL_FRONTBUFFER_BITS_PER_PIPE * I915_MAX_PIPES)
#define INTEL_FRONTBUFFER_PRIMARY(pipe) \
	(1 << (INTEL_FRONTBUFFER_BITS_PER_PIPE * (pipe)))
#define INTEL_FRONTBUFFER_CURSOR(pipe) \
	(1 << (1 +(INTEL_FRONTBUFFER_BITS_PER_PIPE * (pipe))))
#define INTEL_FRONTBUFFER_SPRITE(pipe) \
	(1 << (2 +(INTEL_FRONTBUFFER_BITS_PER_PIPE * (pipe))))
#define INTEL_FRONTBUFFER_OVERLAY(pipe) \
	(1 << (3 +(INTEL_FRONTBUFFER_BITS_PER_PIPE * (pipe))))
#define INTEL_FRONTBUFFER_ALL_MASK(pipe) \
	(0xf << (INTEL_FRONTBUFFER_BITS_PER_PIPE * (pipe)))

struct drm_i915_gem_object {
	struct drm_gem_object base;

	const struct drm_i915_gem_object_ops *ops;

	/** List of VMAs backed by this object */
	struct list_head vma_list;

	/** Stolen memory for this object, instead of being backed by shmem. */
	struct drm_mm_node *stolen;
	struct list_head global_list;

	struct list_head ring_list;
	/** Used in execbuf to temporarily hold a ref */
	struct list_head obj_exec_link;

	/**
	 * This is set if the object is on the active lists (has pending
	 * rendering and so a non-zero seqno), and is not set if it i s on
	 * inactive (ready to be unbound) list.
	 */
	unsigned int active:1;

	/**
	 * This is set if the object has been written to since last bound
	 * to the GTT
	 */
	unsigned int dirty:1;

	/**
	 * Fence register bits (if any) for this object.  Will be set
	 * as needed when mapped into the GTT.
	 * Protected by dev->struct_mutex.
	 */
	signed int fence_reg:I915_MAX_NUM_FENCE_BITS;

	/**
	 * Advice: are the backing pages purgeable?
	 */
	unsigned int madv:2;

	/**
	 * Current tiling mode for the object.
	 */
	unsigned int tiling_mode:2;
	/**
	 * Whether the tiling parameters for the currently associated fence
	 * register have changed. Note that for the purposes of tracking
	 * tiling changes we also treat the unfenced register, the register
	 * slot that the object occupies whilst it executes a fenced
	 * command (such as BLT on gen2/3), as a "fence".
	 */
	unsigned int fence_dirty:1;

	/**
	 * Is the object at the current location in the gtt mappable and
	 * fenceable? Used to avoid costly recalculations.
	 */
	unsigned int map_and_fenceable:1;

	/**
	 * Whether the current gtt mapping needs to be mappable (and isn't just
	 * mappable by accident). Track pin and fault separate for a more
	 * accurate mappable working set.
	 */
	unsigned int fault_mappable:1;
	unsigned int pin_mappable:1;
	unsigned int pin_display:1;

	/*
	 * Is the object to be mapped as read-only to the GPU
	 * Only honoured if hardware has relevant pte bit
	 */
	unsigned long gt_ro:1;
	unsigned int cache_level:3;

	unsigned int has_dma_mapping:1;

	unsigned int frontbuffer_bits:INTEL_FRONTBUFFER_BITS;

	struct sg_table *pages;
	int pages_pin_count;

	/* prime dma-buf support */
	void *dma_buf_vmapping;
	int vmapping_count;

	struct intel_engine_cs *ring;

	/** Breadcrumb of last rendering to the buffer. */
	uint32_t last_read_seqno;
	uint32_t last_write_seqno;
	/** Breadcrumb of last fenced GPU access to the buffer. */
	uint32_t last_fenced_seqno;

	/** Current tiling stride for the object, if it's tiled. */
	uint32_t stride;

	/** References from framebuffers, locks out tiling changes. */
	unsigned long framebuffer_references;

	/** Record of address bit 17 of each page at last unbind. */
	unsigned long *bit_17;

	/** User space pin count and filp owning the pin */
	unsigned long user_pin_count;
	struct drm_file *pin_filp;

	union {
		/** for phy allocated objects */
		struct drm_dma_handle *phys_handle;

		struct i915_gem_userptr {
			uintptr_t ptr;
			unsigned read_only :1;
			unsigned workers :4;
#define I915_GEM_USERPTR_MAX_WORKERS 15

			struct i915_mm_struct *mm;
			struct i915_mmu_object *mmu_object;
			struct work_struct *work;
		} userptr;
	};
};
#define to_intel_bo(x) container_of(x, struct drm_i915_gem_object, base)

void i915_gem_track_fb(struct drm_i915_gem_object *old,
		       struct drm_i915_gem_object *new,
		       unsigned frontbuffer_bits);

/**
 * Request queue structure.
 *
 * The request queue allows us to note sequence numbers that have been emitted
 * and may be associated with active buffers to be retired.
 *
 * By keeping this list, we can avoid having to do questionable
 * sequence-number comparisons on buffer last_rendering_seqnos, and associate
 * an emission time with seqnos for tracking how far ahead of the GPU we are.
 */
struct drm_i915_gem_request {
	/** On Which ring this request was generated */
	struct intel_engine_cs *ring;

	/** GEM sequence number associated with this request. */
	uint32_t seqno;

	/** Position in the ringbuffer of the start of the request */
	u32 head;

	/** Position in the ringbuffer of the end of the request */
	u32 tail;

	/** Context related to this request */
	struct intel_context *ctx;

	/** Batch buffer related to this request if any */
	struct drm_i915_gem_object *batch_obj;

	/** Time at which this request was emitted, in jiffies. */
	unsigned long emitted_jiffies;

	/** global list entry for this request */
	struct list_head list;

	struct drm_i915_file_private *file_priv;
	/** file_priv list entry for this request */
	struct list_head client_list;
};

struct drm_i915_file_private {
	struct drm_i915_private *dev_priv;
	struct drm_file *file;

	struct {
		spinlock_t lock;
		struct list_head request_list;
		struct delayed_work idle_work;
	} mm;
	struct idr context_idr;

	atomic_t rps_wait_boost;
	struct  intel_engine_cs *bsd_ring;
};

/*
 * A command that requires special handling by the command parser.
 */
struct drm_i915_cmd_descriptor {
	/*
	 * Flags describing how the command parser processes the command.
	 *
	 * CMD_DESC_FIXED: The command has a fixed length if this is set,
	 *                 a length mask if not set
	 * CMD_DESC_SKIP: The command is allowed but does not follow the
	 *                standard length encoding for the opcode range in
	 *                which it falls
	 * CMD_DESC_REJECT: The command is never allowed
	 * CMD_DESC_REGISTER: The command should be checked against the
	 *                    register whitelist for the appropriate ring
	 * CMD_DESC_MASTER: The command is allowed if the submitting process
	 *                  is the DRM master
	 */
	u32 flags;
#define CMD_DESC_FIXED    (1<<0)
#define CMD_DESC_SKIP     (1<<1)
#define CMD_DESC_REJECT   (1<<2)
#define CMD_DESC_REGISTER (1<<3)
#define CMD_DESC_BITMASK  (1<<4)
#define CMD_DESC_MASTER   (1<<5)

	/*
	 * The command's unique identification bits and the bitmask to get them.
	 * This isn't strictly the opcode field as defined in the spec and may
	 * also include type, subtype, and/or subop fields.
	 */
	struct {
		u32 value;
		u32 mask;
	} cmd;

	/*
	 * The command's length. The command is either fixed length (i.e. does
	 * not include a length field) or has a length field mask. The flag
	 * CMD_DESC_FIXED indicates a fixed length. Otherwise, the command has
	 * a length mask. All command entries in a command table must include
	 * length information.
	 */
	union {
		u32 fixed;
		u32 mask;
	} length;

	/*
	 * Describes where to find a register address in the command to check
	 * against the ring's register whitelist. Only valid if flags has the
	 * CMD_DESC_REGISTER bit set.
	 */
	struct {
		u32 offset;
		u32 mask;
	} reg;

#define MAX_CMD_DESC_BITMASKS 3
	/*
	 * Describes command checks where a particular dword is masked and
	 * compared against an expected value. If the command does not match
	 * the expected value, the parser rejects it. Only valid if flags has
	 * the CMD_DESC_BITMASK bit set. Only entries where mask is non-zero
	 * are valid.
	 *
	 * If the check specifies a non-zero condition_mask then the parser
	 * only performs the check when the bits specified by condition_mask
	 * are non-zero.
	 */
	struct {
		u32 offset;
		u32 mask;
		u32 expected;
		u32 condition_offset;
		u32 condition_mask;
	} bits[MAX_CMD_DESC_BITMASKS];
};

/*
 * A table of commands requiring special handling by the command parser.
 *
 * Each ring has an array of tables. Each table consists of an array of command
 * descriptors, which must be sorted with command opcodes in ascending order.
 */
struct drm_i915_cmd_table {
	const struct drm_i915_cmd_descriptor *table;
	int count;
};

/* Note that the (struct drm_i915_private *) cast is just to shut up gcc. */
#define __I915__(p) ({ \
	struct drm_i915_private *__p; \
	if (__builtin_types_compatible_p(typeof(*p), struct drm_i915_private)) \
		__p = (struct drm_i915_private *)p; \
	else if (__builtin_types_compatible_p(typeof(*p), struct drm_device)) \
		__p = to_i915((struct drm_device *)p); \
	else \
		BUILD_BUG(); \
	__p; \
})
#define INTEL_INFO(p) 	(&__I915__(p)->info)
#define INTEL_DEVID(p)	(INTEL_INFO(p)->device_id)

#define IS_I830(dev)		(INTEL_DEVID(dev) == 0x3577)
#define IS_845G(dev)		(INTEL_DEVID(dev) == 0x2562)
#define IS_I85X(dev)		(INTEL_INFO(dev)->is_i85x)
#define IS_I865G(dev)		(INTEL_DEVID(dev) == 0x2572)
#define IS_I915G(dev)		(INTEL_INFO(dev)->is_i915g)
#define IS_I915GM(dev)		(INTEL_DEVID(dev) == 0x2592)
#define IS_I945G(dev)		(INTEL_DEVID(dev) == 0x2772)
#define IS_I945GM(dev)		(INTEL_INFO(dev)->is_i945gm)
#define IS_BROADWATER(dev)	(INTEL_INFO(dev)->is_broadwater)
#define IS_CRESTLINE(dev)	(INTEL_INFO(dev)->is_crestline)
#define IS_GM45(dev)		(INTEL_DEVID(dev) == 0x2A42)
#define IS_G4X(dev)		(INTEL_INFO(dev)->is_g4x)
#define IS_PINEVIEW_G(dev)	(INTEL_DEVID(dev) == 0xa001)
#define IS_PINEVIEW_M(dev)	(INTEL_DEVID(dev) == 0xa011)
#define IS_PINEVIEW(dev)	(INTEL_INFO(dev)->is_pineview)
#define IS_G33(dev)		(INTEL_INFO(dev)->is_g33)
#define IS_IRONLAKE_M(dev)	(INTEL_DEVID(dev) == 0x0046)
#define IS_IVYBRIDGE(dev)	(INTEL_INFO(dev)->is_ivybridge)
#define IS_IVB_GT1(dev)		(INTEL_DEVID(dev) == 0x0156 || \
				 INTEL_DEVID(dev) == 0x0152 || \
				 INTEL_DEVID(dev) == 0x015a)
#define IS_SNB_GT1(dev)		(INTEL_DEVID(dev) == 0x0102 || \
				 INTEL_DEVID(dev) == 0x0106 || \
				 INTEL_DEVID(dev) == 0x010A)
#define IS_VALLEYVIEW(dev)	(INTEL_INFO(dev)->is_valleyview)
#define IS_CHERRYVIEW(dev)	(INTEL_INFO(dev)->is_valleyview && IS_GEN8(dev))
#define IS_HASWELL(dev)	(INTEL_INFO(dev)->is_haswell)
#define IS_BROADWELL(dev)	(!INTEL_INFO(dev)->is_valleyview && IS_GEN8(dev))
#define IS_SKYLAKE(dev)	(INTEL_INFO(dev)->is_skylake)
#define IS_MOBILE(dev)		(INTEL_INFO(dev)->is_mobile)
#define IS_HSW_EARLY_SDV(dev)	(IS_HASWELL(dev) && \
				 (INTEL_DEVID(dev) & 0xFF00) == 0x0C00)
#define IS_BDW_ULT(dev)		(IS_BROADWELL(dev) && \
				 ((INTEL_DEVID(dev) & 0xf) == 0x2  || \
				 (INTEL_DEVID(dev) & 0xf) == 0x6 || \
				 (INTEL_DEVID(dev) & 0xf) == 0xe))
#define IS_BDW_GT3(dev)		(IS_BROADWELL(dev) && \
				 (INTEL_DEVID(dev) & 0x00F0) == 0x0020)
#define IS_HSW_ULT(dev)		(IS_HASWELL(dev) && \
				 (INTEL_DEVID(dev) & 0xFF00) == 0x0A00)
#define IS_HSW_GT3(dev)		(IS_HASWELL(dev) && \
				 (INTEL_DEVID(dev) & 0x00F0) == 0x0020)
/* ULX machines are also considered ULT. */
#define IS_HSW_ULX(dev)		(INTEL_DEVID(dev) == 0x0A0E || \
				 INTEL_DEVID(dev) == 0x0A1E)
#define IS_PRELIMINARY_HW(intel_info) ((intel_info)->is_preliminary)

/*
 * The genX designation typically refers to the render engine, so render
 * capability related checks should use IS_GEN, while display and other checks
 * have their own (e.g. HAS_PCH_SPLIT for ILK+ display, IS_foo for particular
 * chips, etc.).
 */
#define IS_GEN2(dev)	(INTEL_INFO(dev)->gen == 2)
#define IS_GEN3(dev)	(INTEL_INFO(dev)->gen == 3)
#define IS_GEN4(dev)	(INTEL_INFO(dev)->gen == 4)
#define IS_GEN5(dev)	(INTEL_INFO(dev)->gen == 5)
#define IS_GEN6(dev)	(INTEL_INFO(dev)->gen == 6)
#define IS_GEN7(dev)	(INTEL_INFO(dev)->gen == 7)
#define IS_GEN8(dev)	(INTEL_INFO(dev)->gen == 8)
#define IS_GEN9(dev)	(INTEL_INFO(dev)->gen == 9)

#define RENDER_RING		(1<<RCS)
#define BSD_RING		(1<<VCS)
#define BLT_RING		(1<<BCS)
#define VEBOX_RING		(1<<VECS)
#define BSD2_RING		(1<<VCS2)
#define HAS_BSD(dev)		(INTEL_INFO(dev)->ring_mask & BSD_RING)
#define HAS_BSD2(dev)		(INTEL_INFO(dev)->ring_mask & BSD2_RING)
#define HAS_BLT(dev)		(INTEL_INFO(dev)->ring_mask & BLT_RING)
#define HAS_VEBOX(dev)		(INTEL_INFO(dev)->ring_mask & VEBOX_RING)
#define HAS_LLC(dev)		(INTEL_INFO(dev)->has_llc)
#define HAS_WT(dev)		((IS_HASWELL(dev) || IS_BROADWELL(dev)) && \
				 __I915__(dev)->ellc_size)
#define I915_NEED_GFX_HWS(dev)	(INTEL_INFO(dev)->need_gfx_hws)

#define HAS_HW_CONTEXTS(dev)	(INTEL_INFO(dev)->gen >= 6)
#define HAS_LOGICAL_RING_CONTEXTS(dev)	(INTEL_INFO(dev)->gen >= 8)
#define USES_PPGTT(dev)		(i915.enable_ppgtt)
#define USES_FULL_PPGTT(dev)	(i915.enable_ppgtt == 2)

#define HAS_OVERLAY(dev)		(INTEL_INFO(dev)->has_overlay)
#define OVERLAY_NEEDS_PHYSICAL(dev)	(INTEL_INFO(dev)->overlay_needs_physical)

/* Early gen2 have a totally busted CS tlb and require pinned batches. */
#define HAS_BROKEN_CS_TLB(dev)		(IS_I830(dev) || IS_845G(dev))
/*
 * dp aux and gmbus irq on gen4 seems to be able to generate legacy interrupts
 * even when in MSI mode. This results in spurious interrupt warnings if the
 * legacy irq no. is shared with another device. The kernel then disables that
 * interrupt source and so prevents the other device from working properly.
 */
#define HAS_AUX_IRQ(dev) (INTEL_INFO(dev)->gen >= 5)
#define HAS_GMBUS_IRQ(dev) (INTEL_INFO(dev)->gen >= 5)

/* With the 945 and later, Y tiling got adjusted so that it was 32 128-byte
 * rows, which changed the alignment requirements and fence programming.
 */
#define HAS_128_BYTE_Y_TILING(dev) (!IS_GEN2(dev) && !(IS_I915G(dev) || \
						      IS_I915GM(dev)))
#define SUPPORTS_DIGITAL_OUTPUTS(dev)	(!IS_GEN2(dev) && !IS_PINEVIEW(dev))
#define SUPPORTS_INTEGRATED_HDMI(dev)	(IS_G4X(dev) || IS_GEN5(dev))
#define SUPPORTS_INTEGRATED_DP(dev)	(IS_G4X(dev) || IS_GEN5(dev))
#define SUPPORTS_TV(dev)		(INTEL_INFO(dev)->supports_tv)
#define I915_HAS_HOTPLUG(dev)		 (INTEL_INFO(dev)->has_hotplug)

#define HAS_FW_BLC(dev) (INTEL_INFO(dev)->gen > 2)
#define HAS_PIPE_CXSR(dev) (INTEL_INFO(dev)->has_pipe_cxsr)
#define HAS_FBC(dev) (INTEL_INFO(dev)->has_fbc)

#define HAS_IPS(dev)		(IS_HSW_ULT(dev) || IS_BROADWELL(dev))

#define HAS_DDI(dev)		(INTEL_INFO(dev)->has_ddi)
#define HAS_FPGA_DBG_UNCLAIMED(dev)	(INTEL_INFO(dev)->has_fpga_dbg)
#define HAS_PSR(dev)		(IS_HASWELL(dev) || IS_BROADWELL(dev))
#define HAS_RUNTIME_PM(dev)	(IS_GEN6(dev) || IS_HASWELL(dev) || \
				 IS_BROADWELL(dev) || IS_VALLEYVIEW(dev))
#define HAS_RC6(dev)		(INTEL_INFO(dev)->gen >= 6)
#define HAS_RC6p(dev)		(INTEL_INFO(dev)->gen == 6 || IS_IVYBRIDGE(dev))

#define INTEL_PCH_DEVICE_ID_MASK		0xff00
#define INTEL_PCH_IBX_DEVICE_ID_TYPE		0x3b00
#define INTEL_PCH_CPT_DEVICE_ID_TYPE		0x1c00
#define INTEL_PCH_PPT_DEVICE_ID_TYPE		0x1e00
#define INTEL_PCH_LPT_DEVICE_ID_TYPE		0x8c00
#define INTEL_PCH_LPT_LP_DEVICE_ID_TYPE		0x9c00
#define INTEL_PCH_SPT_DEVICE_ID_TYPE		0xA100
#define INTEL_PCH_SPT_LP_DEVICE_ID_TYPE		0x9D00

#define INTEL_PCH_TYPE(dev) (__I915__(dev)->pch_type)
#define HAS_PCH_SPT(dev) (INTEL_PCH_TYPE(dev) == PCH_SPT)
#define HAS_PCH_LPT(dev) (INTEL_PCH_TYPE(dev) == PCH_LPT)
#define HAS_PCH_CPT(dev) (INTEL_PCH_TYPE(dev) == PCH_CPT)
#define HAS_PCH_IBX(dev) (INTEL_PCH_TYPE(dev) == PCH_IBX)
#define HAS_PCH_NOP(dev) (INTEL_PCH_TYPE(dev) == PCH_NOP)
#define HAS_PCH_SPLIT(dev) (INTEL_PCH_TYPE(dev) != PCH_NONE)

#define HAS_GMCH_DISPLAY(dev) (INTEL_INFO(dev)->gen < 5 || IS_VALLEYVIEW(dev))

/* DPF == dynamic parity feature */
#define HAS_L3_DPF(dev) (IS_IVYBRIDGE(dev) || IS_HASWELL(dev))
#define NUM_L3_SLICES(dev) (IS_HSW_GT3(dev) ? 2 : HAS_L3_DPF(dev))

#define GT_FREQUENCY_MULTIPLIER 50

#include "i915_trace.h"

extern const struct drm_ioctl_desc i915_ioctls[];
extern int i915_max_ioctl;

extern int i915_suspend_legacy(struct drm_device *dev, pm_message_t state);
extern int i915_resume_legacy(struct drm_device *dev);
extern int i915_master_create(struct drm_device *dev, struct drm_master *master);
extern void i915_master_destroy(struct drm_device *dev, struct drm_master *master);

/* i915_params.c */
struct i915_params {
	int modeset;
	int panel_ignore_lid;
	unsigned int powersave;
	int semaphores;
	unsigned int lvds_downclock;
	int lvds_channel_mode;
	int panel_use_ssc;
	int vbt_sdvo_panel_type;
	int enable_rc6;
	int enable_fbc;
	int enable_ppgtt;
	int enable_execlists;
	int enable_psr;
	unsigned int preliminary_hw_support;
	int disable_power_well;
	int enable_ips;
	int invert_brightness;
	int enable_cmd_parser;
	/* leave bools at the end to not create holes */
	bool enable_hangcheck;
	bool fastboot;
	bool prefault_disable;
	bool reset;
	bool disable_display;
	bool disable_vtd_wa;
	int use_mmio_flip;
	bool mmio_debug;
};
extern struct i915_params i915 __read_mostly;

				/* i915_dma.c */
extern int i915_driver_load(struct drm_device *, unsigned long flags);
extern int i915_driver_unload(struct drm_device *);
extern int i915_driver_open(struct drm_device *dev, struct drm_file *file);
extern void i915_driver_lastclose(struct drm_device * dev);
extern void i915_driver_preclose(struct drm_device *dev,
				 struct drm_file *file);
extern void i915_driver_postclose(struct drm_device *dev,
				  struct drm_file *file);
extern int i915_driver_device_is_agp(struct drm_device * dev);
#ifdef CONFIG_COMPAT
extern long i915_compat_ioctl(struct file *filp, unsigned int cmd,
			      unsigned long arg);
#endif
extern int intel_gpu_reset(struct drm_device *dev);
extern int i915_reset(struct drm_device *dev);
extern unsigned long i915_chipset_val(struct drm_i915_private *dev_priv);
extern unsigned long i915_mch_val(struct drm_i915_private *dev_priv);
extern unsigned long i915_gfx_val(struct drm_i915_private *dev_priv);
extern void i915_update_gfx_val(struct drm_i915_private *dev_priv);
int vlv_force_gfx_clock(struct drm_i915_private *dev_priv, bool on);
void intel_hpd_cancel_work(struct drm_i915_private *dev_priv);

/* i915_irq.c */
void i915_queue_hangcheck(struct drm_device *dev);
__printf(3, 4)
void i915_handle_error(struct drm_device *dev, bool wedged,
		       const char *fmt, ...);

extern void intel_irq_init(struct drm_i915_private *dev_priv);
extern void intel_hpd_init(struct drm_i915_private *dev_priv);
int intel_irq_install(struct drm_i915_private *dev_priv);
void intel_irq_uninstall(struct drm_i915_private *dev_priv);

extern void intel_uncore_sanitize(struct drm_device *dev);
extern void intel_uncore_early_sanitize(struct drm_device *dev,
					bool restore_forcewake);
extern void intel_uncore_init(struct drm_device *dev);
extern void intel_uncore_check_errors(struct drm_device *dev);
extern void intel_uncore_fini(struct drm_device *dev);
extern void intel_uncore_forcewake_reset(struct drm_device *dev, bool restore);

void
i915_enable_pipestat(struct drm_i915_private *dev_priv, enum pipe pipe,
		     u32 status_mask);

void
i915_disable_pipestat(struct drm_i915_private *dev_priv, enum pipe pipe,
		      u32 status_mask);

void valleyview_enable_display_irqs(struct drm_i915_private *dev_priv);
void valleyview_disable_display_irqs(struct drm_i915_private *dev_priv);
void
ironlake_enable_display_irq(struct drm_i915_private *dev_priv, u32 mask);
void
ironlake_disable_display_irq(struct drm_i915_private *dev_priv, u32 mask);
void ibx_display_interrupt_update(struct drm_i915_private *dev_priv,
				  uint32_t interrupt_mask,
				  uint32_t enabled_irq_mask);
#define ibx_enable_display_interrupt(dev_priv, bits) \
	ibx_display_interrupt_update((dev_priv), (bits), (bits))
#define ibx_disable_display_interrupt(dev_priv, bits) \
	ibx_display_interrupt_update((dev_priv), (bits), 0)

/* i915_gem.c */
int i915_gem_create_ioctl(struct drm_device *dev, void *data,
			  struct drm_file *file_priv);
int i915_gem_pread_ioctl(struct drm_device *dev, void *data,
			 struct drm_file *file_priv);
int i915_gem_pwrite_ioctl(struct drm_device *dev, void *data,
			  struct drm_file *file_priv);
int i915_gem_mmap_ioctl(struct drm_device *dev, void *data,
			struct drm_file *file_priv);
int i915_gem_mmap_gtt_ioctl(struct drm_device *dev, void *data,
			struct drm_file *file_priv);
int i915_gem_set_domain_ioctl(struct drm_device *dev, void *data,
			      struct drm_file *file_priv);
int i915_gem_sw_finish_ioctl(struct drm_device *dev, void *data,
			     struct drm_file *file_priv);
void i915_gem_execbuffer_move_to_active(struct list_head *vmas,
					struct intel_engine_cs *ring);
void i915_gem_execbuffer_retire_commands(struct drm_device *dev,
					 struct drm_file *file,
					 struct intel_engine_cs *ring,
					 struct drm_i915_gem_object *obj);
int i915_gem_ringbuffer_submission(struct drm_device *dev,
				   struct drm_file *file,
				   struct intel_engine_cs *ring,
				   struct intel_context *ctx,
				   struct drm_i915_gem_execbuffer2 *args,
				   struct list_head *vmas,
				   struct drm_i915_gem_object *batch_obj,
				   u64 exec_start, u32 flags);
int i915_gem_execbuffer(struct drm_device *dev, void *data,
			struct drm_file *file_priv);
int i915_gem_execbuffer2(struct drm_device *dev, void *data,
			 struct drm_file *file_priv);
int i915_gem_pin_ioctl(struct drm_device *dev, void *data,
		       struct drm_file *file_priv);
int i915_gem_unpin_ioctl(struct drm_device *dev, void *data,
			 struct drm_file *file_priv);
int i915_gem_busy_ioctl(struct drm_device *dev, void *data,
			struct drm_file *file_priv);
int i915_gem_get_caching_ioctl(struct drm_device *dev, void *data,
			       struct drm_file *file);
int i915_gem_set_caching_ioctl(struct drm_device *dev, void *data,
			       struct drm_file *file);
int i915_gem_throttle_ioctl(struct drm_device *dev, void *data,
			    struct drm_file *file_priv);
int i915_gem_madvise_ioctl(struct drm_device *dev, void *data,
			   struct drm_file *file_priv);
int i915_gem_set_tiling(struct drm_device *dev, void *data,
			struct drm_file *file_priv);
int i915_gem_get_tiling(struct drm_device *dev, void *data,
			struct drm_file *file_priv);
int i915_gem_init_userptr(struct drm_device *dev);
int i915_gem_userptr_ioctl(struct drm_device *dev, void *data,
			   struct drm_file *file);
int i915_gem_get_aperture_ioctl(struct drm_device *dev, void *data,
				struct drm_file *file_priv);
int i915_gem_wait_ioctl(struct drm_device *dev, void *data,
			struct drm_file *file_priv);
void i915_gem_load(struct drm_device *dev);
unsigned long i915_gem_shrink(struct drm_i915_private *dev_priv,
			      long target,
			      unsigned flags);
#define I915_SHRINK_PURGEABLE 0x1
#define I915_SHRINK_UNBOUND 0x2
#define I915_SHRINK_BOUND 0x4
void *i915_gem_object_alloc(struct drm_device *dev);
void i915_gem_object_free(struct drm_i915_gem_object *obj);
void i915_gem_object_init(struct drm_i915_gem_object *obj,
			 const struct drm_i915_gem_object_ops *ops);
struct drm_i915_gem_object *i915_gem_alloc_object(struct drm_device *dev,
						  size_t size);
void i915_init_vm(struct drm_i915_private *dev_priv,
		  struct i915_address_space *vm);
void i915_gem_free_object(struct drm_gem_object *obj);
void i915_gem_vma_destroy(struct i915_vma *vma);

#define PIN_MAPPABLE 0x1
#define PIN_NONBLOCK 0x2
#define PIN_GLOBAL 0x4
#define PIN_OFFSET_BIAS 0x8
#define PIN_OFFSET_MASK (~4095)
int __must_check i915_gem_object_pin(struct drm_i915_gem_object *obj,
				     struct i915_address_space *vm,
				     uint32_t alignment,
				     uint64_t flags);
int __must_check i915_vma_unbind(struct i915_vma *vma);
int i915_gem_object_put_pages(struct drm_i915_gem_object *obj);
void i915_gem_release_all_mmaps(struct drm_i915_private *dev_priv);
void i915_gem_release_mmap(struct drm_i915_gem_object *obj);

int i915_gem_obj_prepare_shmem_read(struct drm_i915_gem_object *obj,
				    int *needs_clflush);

int __must_check i915_gem_object_get_pages(struct drm_i915_gem_object *obj);
static inline struct page *i915_gem_object_get_page(struct drm_i915_gem_object *obj, int n)
{
	struct sg_page_iter sg_iter;

	for_each_sg_page(obj->pages->sgl, &sg_iter, obj->pages->nents, n)
		return sg_page_iter_page(&sg_iter);

	return NULL;
}
static inline void i915_gem_object_pin_pages(struct drm_i915_gem_object *obj)
{
	BUG_ON(obj->pages == NULL);
	obj->pages_pin_count++;
}
static inline void i915_gem_object_unpin_pages(struct drm_i915_gem_object *obj)
{
	BUG_ON(obj->pages_pin_count == 0);
	obj->pages_pin_count--;
}

int __must_check i915_mutex_lock_interruptible(struct drm_device *dev);
int i915_gem_object_sync(struct drm_i915_gem_object *obj,
			 struct intel_engine_cs *to);
void i915_vma_move_to_active(struct i915_vma *vma,
			     struct intel_engine_cs *ring);
int i915_gem_dumb_create(struct drm_file *file_priv,
			 struct drm_device *dev,
			 struct drm_mode_create_dumb *args);
int i915_gem_mmap_gtt(struct drm_file *file_priv, struct drm_device *dev,
		      uint32_t handle, uint64_t *offset);
/**
 * Returns true if seq1 is later than seq2.
 */
static inline bool
i915_seqno_passed(uint32_t seq1, uint32_t seq2)
{
	return (int32_t)(seq1 - seq2) >= 0;
}

int __must_check i915_gem_get_seqno(struct drm_device *dev, u32 *seqno);
int __must_check i915_gem_set_seqno(struct drm_device *dev, u32 seqno);
int __must_check i915_gem_object_get_fence(struct drm_i915_gem_object *obj);
int __must_check i915_gem_object_put_fence(struct drm_i915_gem_object *obj);

bool i915_gem_object_pin_fence(struct drm_i915_gem_object *obj);
void i915_gem_object_unpin_fence(struct drm_i915_gem_object *obj);

struct drm_i915_gem_request *
i915_gem_find_active_request(struct intel_engine_cs *ring);

bool i915_gem_retire_requests(struct drm_device *dev);
void i915_gem_retire_requests_ring(struct intel_engine_cs *ring);
int __must_check i915_gem_check_wedge(struct i915_gpu_error *error,
				      bool interruptible);
int __must_check i915_gem_check_olr(struct intel_engine_cs *ring, u32 seqno);

static inline bool i915_reset_in_progress(struct i915_gpu_error *error)
{
	return unlikely(atomic_read(&error->reset_counter)
			& (I915_RESET_IN_PROGRESS_FLAG | I915_WEDGED));
}

static inline bool i915_terminally_wedged(struct i915_gpu_error *error)
{
	return atomic_read(&error->reset_counter) & I915_WEDGED;
}

static inline u32 i915_reset_count(struct i915_gpu_error *error)
{
	return ((atomic_read(&error->reset_counter) & ~I915_WEDGED) + 1) / 2;
}

static inline bool i915_stop_ring_allow_ban(struct drm_i915_private *dev_priv)
{
	return dev_priv->gpu_error.stop_rings == 0 ||
		dev_priv->gpu_error.stop_rings & I915_STOP_RING_ALLOW_BAN;
}

static inline bool i915_stop_ring_allow_warn(struct drm_i915_private *dev_priv)
{
	return dev_priv->gpu_error.stop_rings == 0 ||
		dev_priv->gpu_error.stop_rings & I915_STOP_RING_ALLOW_WARN;
}

void i915_gem_reset(struct drm_device *dev);
bool i915_gem_clflush_object(struct drm_i915_gem_object *obj, bool force);
int __must_check i915_gem_object_finish_gpu(struct drm_i915_gem_object *obj);
int __must_check i915_gem_init(struct drm_device *dev);
int i915_gem_init_rings(struct drm_device *dev);
int __must_check i915_gem_init_hw(struct drm_device *dev);
int i915_gem_l3_remap(struct intel_engine_cs *ring, int slice);
void i915_gem_init_swizzling(struct drm_device *dev);
void i915_gem_cleanup_ringbuffer(struct drm_device *dev);
int __must_check i915_gpu_idle(struct drm_device *dev);
int __must_check i915_gem_suspend(struct drm_device *dev);
int __i915_add_request(struct intel_engine_cs *ring,
		       struct drm_file *file,
		       struct drm_i915_gem_object *batch_obj,
		       u32 *seqno);
#define i915_add_request(ring, seqno) \
	__i915_add_request(ring, NULL, NULL, seqno)
int __i915_wait_seqno(struct intel_engine_cs *ring, u32 seqno,
			unsigned reset_counter,
			bool interruptible,
			s64 *timeout,
			struct drm_i915_file_private *file_priv);
int __must_check i915_wait_seqno(struct intel_engine_cs *ring,
				 uint32_t seqno);
int i915_gem_fault(struct vm_area_struct *vma, struct vm_fault *vmf);
int __must_check
i915_gem_object_set_to_gtt_domain(struct drm_i915_gem_object *obj,
				  bool write);
int __must_check
i915_gem_object_set_to_cpu_domain(struct drm_i915_gem_object *obj, bool write);
int __must_check
i915_gem_object_pin_to_display_plane(struct drm_i915_gem_object *obj,
				     u32 alignment,
				     struct intel_engine_cs *pipelined);
void i915_gem_object_unpin_from_display_plane(struct drm_i915_gem_object *obj);
int i915_gem_object_attach_phys(struct drm_i915_gem_object *obj,
				int align);
int i915_gem_open(struct drm_device *dev, struct drm_file *file);
void i915_gem_release(struct drm_device *dev, struct drm_file *file);

uint32_t
i915_gem_get_gtt_size(struct drm_device *dev, uint32_t size, int tiling_mode);
uint32_t
i915_gem_get_gtt_alignment(struct drm_device *dev, uint32_t size,
			    int tiling_mode, bool fenced);

int i915_gem_object_set_cache_level(struct drm_i915_gem_object *obj,
				    enum i915_cache_level cache_level);

struct drm_gem_object *i915_gem_prime_import(struct drm_device *dev,
				struct dma_buf *dma_buf);

struct dma_buf *i915_gem_prime_export(struct drm_device *dev,
				struct drm_gem_object *gem_obj, int flags);

void i915_gem_restore_fences(struct drm_device *dev);

unsigned long i915_gem_obj_offset(struct drm_i915_gem_object *o,
				  struct i915_address_space *vm);
bool i915_gem_obj_bound_any(struct drm_i915_gem_object *o);
bool i915_gem_obj_bound(struct drm_i915_gem_object *o,
			struct i915_address_space *vm);
unsigned long i915_gem_obj_size(struct drm_i915_gem_object *o,
				struct i915_address_space *vm);
struct i915_vma *i915_gem_obj_to_vma(struct drm_i915_gem_object *obj,
				     struct i915_address_space *vm);
struct i915_vma *
i915_gem_obj_lookup_or_create_vma(struct drm_i915_gem_object *obj,
				  struct i915_address_space *vm);

struct i915_vma *i915_gem_obj_to_ggtt(struct drm_i915_gem_object *obj);
static inline bool i915_gem_obj_is_pinned(struct drm_i915_gem_object *obj) {
	struct i915_vma *vma;
	list_for_each_entry(vma, &obj->vma_list, vma_link)
		if (vma->pin_count > 0)
			return true;
	return false;
}

/* Some GGTT VM helpers */
#define i915_obj_to_ggtt(obj) \
	(&((struct drm_i915_private *)(obj)->base.dev->dev_private)->gtt.base)
static inline bool i915_is_ggtt(struct i915_address_space *vm)
{
	struct i915_address_space *ggtt =
		&((struct drm_i915_private *)(vm)->dev->dev_private)->gtt.base;
	return vm == ggtt;
}

static inline struct i915_hw_ppgtt *
i915_vm_to_ppgtt(struct i915_address_space *vm)
{
	WARN_ON(i915_is_ggtt(vm));

	return container_of(vm, struct i915_hw_ppgtt, base);
}


static inline bool i915_gem_obj_ggtt_bound(struct drm_i915_gem_object *obj)
{
	return i915_gem_obj_bound(obj, i915_obj_to_ggtt(obj));
}

static inline unsigned long
i915_gem_obj_ggtt_offset(struct drm_i915_gem_object *obj)
{
	return i915_gem_obj_offset(obj, i915_obj_to_ggtt(obj));
}

static inline unsigned long
i915_gem_obj_ggtt_size(struct drm_i915_gem_object *obj)
{
	return i915_gem_obj_size(obj, i915_obj_to_ggtt(obj));
}

static inline int __must_check
i915_gem_obj_ggtt_pin(struct drm_i915_gem_object *obj,
		      uint32_t alignment,
		      unsigned flags)
{
	return i915_gem_object_pin(obj, i915_obj_to_ggtt(obj),
				   alignment, flags | PIN_GLOBAL);
}

static inline int
i915_gem_object_ggtt_unbind(struct drm_i915_gem_object *obj)
{
	return i915_vma_unbind(i915_gem_obj_to_ggtt(obj));
}

void i915_gem_object_ggtt_unpin(struct drm_i915_gem_object *obj);

/* i915_gem_context.c */
int __must_check i915_gem_context_init(struct drm_device *dev);
void i915_gem_context_fini(struct drm_device *dev);
void i915_gem_context_reset(struct drm_device *dev);
int i915_gem_context_open(struct drm_device *dev, struct drm_file *file);
int i915_gem_context_enable(struct drm_i915_private *dev_priv);
void i915_gem_context_close(struct drm_device *dev, struct drm_file *file);
int i915_switch_context(struct intel_engine_cs *ring,
			struct intel_context *to);
struct intel_context *
i915_gem_context_get(struct drm_i915_file_private *file_priv, u32 id);
void i915_gem_context_free(struct kref *ctx_ref);
struct drm_i915_gem_object *
i915_gem_alloc_context_obj(struct drm_device *dev, size_t size);
static inline void i915_gem_context_reference(struct intel_context *ctx)
{
	kref_get(&ctx->ref);
}

static inline void i915_gem_context_unreference(struct intel_context *ctx)
{
	kref_put(&ctx->ref, i915_gem_context_free);
}

static inline bool i915_gem_context_is_default(const struct intel_context *c)
{
	return c->user_handle == DEFAULT_CONTEXT_HANDLE;
}

int i915_gem_context_create_ioctl(struct drm_device *dev, void *data,
				  struct drm_file *file);
int i915_gem_context_destroy_ioctl(struct drm_device *dev, void *data,
				   struct drm_file *file);

/* i915_gem_evict.c */
int __must_check i915_gem_evict_something(struct drm_device *dev,
					  struct i915_address_space *vm,
					  int min_size,
					  unsigned alignment,
					  unsigned cache_level,
					  unsigned long start,
					  unsigned long end,
					  unsigned flags);
int i915_gem_evict_vm(struct i915_address_space *vm, bool do_idle);
int i915_gem_evict_everything(struct drm_device *dev);

/* belongs in i915_gem_gtt.h */
static inline void i915_gem_chipset_flush(struct drm_device *dev)
{
	if (INTEL_INFO(dev)->gen < 6)
		intel_gtt_chipset_flush();
}

/* i915_gem_stolen.c */
int i915_gem_init_stolen(struct drm_device *dev);
int i915_gem_stolen_setup_compression(struct drm_device *dev, int size, int fb_cpp);
void i915_gem_stolen_cleanup_compression(struct drm_device *dev);
void i915_gem_cleanup_stolen(struct drm_device *dev);
struct drm_i915_gem_object *
i915_gem_object_create_stolen(struct drm_device *dev, u32 size);
struct drm_i915_gem_object *
i915_gem_object_create_stolen_for_preallocated(struct drm_device *dev,
					       u32 stolen_offset,
					       u32 gtt_offset,
					       u32 size);

/* i915_gem_tiling.c */
static inline bool i915_gem_object_needs_bit17_swizzle(struct drm_i915_gem_object *obj)
{
	struct drm_i915_private *dev_priv = obj->base.dev->dev_private;

	return dev_priv->mm.bit_6_swizzle_x == I915_BIT_6_SWIZZLE_9_10_17 &&
		obj->tiling_mode != I915_TILING_NONE;
}

void i915_gem_detect_bit_6_swizzle(struct drm_device *dev);
void i915_gem_object_do_bit_17_swizzle(struct drm_i915_gem_object *obj);
void i915_gem_object_save_bit_17_swizzle(struct drm_i915_gem_object *obj);

/* i915_gem_debug.c */
#if WATCH_LISTS
int i915_verify_lists(struct drm_device *dev);
#else
#define i915_verify_lists(dev) 0
#endif

/* i915_debugfs.c */
int i915_debugfs_init(struct drm_minor *minor);
void i915_debugfs_cleanup(struct drm_minor *minor);
#ifdef CONFIG_DEBUG_FS
void intel_display_crc_init(struct drm_device *dev);
#else
static inline void intel_display_crc_init(struct drm_device *dev) {}
#endif

/* i915_gpu_error.c */
__printf(2, 3)
void i915_error_printf(struct drm_i915_error_state_buf *e, const char *f, ...);
int i915_error_state_to_str(struct drm_i915_error_state_buf *estr,
			    const struct i915_error_state_file_priv *error);
int i915_error_state_buf_init(struct drm_i915_error_state_buf *eb,
			      struct drm_i915_private *i915,
			      size_t count, loff_t pos);
static inline void i915_error_state_buf_release(
	struct drm_i915_error_state_buf *eb)
{
	kfree(eb->buf);
}
void i915_capture_error_state(struct drm_device *dev, bool wedge,
			      const char *error_msg);
void i915_error_state_get(struct drm_device *dev,
			  struct i915_error_state_file_priv *error_priv);
void i915_error_state_put(struct i915_error_state_file_priv *error_priv);
void i915_destroy_error_state(struct drm_device *dev);

void i915_get_extra_instdone(struct drm_device *dev, uint32_t *instdone);
const char *i915_cache_level_str(struct drm_i915_private *i915, int type);

/* i915_cmd_parser.c */
int i915_cmd_parser_get_version(void);
int i915_cmd_parser_init_ring(struct intel_engine_cs *ring);
void i915_cmd_parser_fini_ring(struct intel_engine_cs *ring);
bool i915_needs_cmd_parser(struct intel_engine_cs *ring);
int i915_parse_cmds(struct intel_engine_cs *ring,
		    struct drm_i915_gem_object *batch_obj,
		    u32 batch_start_offset,
		    bool is_master);

/* i915_suspend.c */
extern int i915_save_state(struct drm_device *dev);
extern int i915_restore_state(struct drm_device *dev);

/* i915_ums.c */
void i915_save_display_reg(struct drm_device *dev);
void i915_restore_display_reg(struct drm_device *dev);

/* i915_sysfs.c */
void i915_setup_sysfs(struct drm_device *dev_priv);
void i915_teardown_sysfs(struct drm_device *dev_priv);

/* intel_i2c.c */
extern int intel_setup_gmbus(struct drm_device *dev);
extern void intel_teardown_gmbus(struct drm_device *dev);
static inline bool intel_gmbus_is_port_valid(unsigned port)
{
	return (port >= GMBUS_PORT_SSC && port <= GMBUS_PORT_DPD);
}

extern struct i2c_adapter *intel_gmbus_get_adapter(
		struct drm_i915_private *dev_priv, unsigned port);
extern void intel_gmbus_set_speed(struct i2c_adapter *adapter, int speed);
extern void intel_gmbus_force_bit(struct i2c_adapter *adapter, bool force_bit);
static inline bool intel_gmbus_is_forced_bit(struct i2c_adapter *adapter)
{
	return container_of(adapter, struct intel_gmbus, adapter)->force_bit;
}
extern void intel_i2c_reset(struct drm_device *dev);

/* intel_opregion.c */
#ifdef CONFIG_ACPI
extern int intel_opregion_setup(struct drm_device *dev);
extern void intel_opregion_init(struct drm_device *dev);
extern void intel_opregion_fini(struct drm_device *dev);
extern void intel_opregion_asle_intr(struct drm_device *dev);
extern int intel_opregion_notify_encoder(struct intel_encoder *intel_encoder,
					 bool enable);
extern int intel_opregion_notify_adapter(struct drm_device *dev,
					 pci_power_t state);
#else
static inline int intel_opregion_setup(struct drm_device *dev) { return 0; }
static inline void intel_opregion_init(struct drm_device *dev) { return; }
static inline void intel_opregion_fini(struct drm_device *dev) { return; }
static inline void intel_opregion_asle_intr(struct drm_device *dev) { return; }
static inline int
intel_opregion_notify_encoder(struct intel_encoder *intel_encoder, bool enable)
{
	return 0;
}
static inline int
intel_opregion_notify_adapter(struct drm_device *dev, pci_power_t state)
{
	return 0;
}
#endif

/* intel_acpi.c */
#ifdef CONFIG_ACPI
extern void intel_register_dsm_handler(void);
extern void intel_unregister_dsm_handler(void);
#else
static inline void intel_register_dsm_handler(void) { return; }
static inline void intel_unregister_dsm_handler(void) { return; }
#endif /* CONFIG_ACPI */

/* modesetting */
extern void intel_modeset_init_hw(struct drm_device *dev);
extern void intel_modeset_init(struct drm_device *dev);
extern void intel_modeset_gem_init(struct drm_device *dev);
extern void intel_modeset_cleanup(struct drm_device *dev);
extern void intel_connector_unregister(struct intel_connector *);
extern int intel_modeset_vga_set_state(struct drm_device *dev, bool state);
extern void intel_modeset_setup_hw_state(struct drm_device *dev,
					 bool force_restore);
extern void i915_redisable_vga(struct drm_device *dev);
extern void i915_redisable_vga_power_on(struct drm_device *dev);
extern bool intel_fbc_enabled(struct drm_device *dev);
extern void bdw_fbc_sw_flush(struct drm_device *dev, u32 value);
extern void intel_disable_fbc(struct drm_device *dev);
extern bool ironlake_set_drps(struct drm_device *dev, u8 val);
extern void intel_init_pch_refclk(struct drm_device *dev);
extern void gen6_set_rps(struct drm_device *dev, u8 val);
extern void valleyview_set_rps(struct drm_device *dev, u8 val);
extern void intel_set_memory_cxsr(struct drm_i915_private *dev_priv,
				  bool enable);
extern void intel_detect_pch(struct drm_device *dev);
extern int intel_trans_dp_port_sel(struct drm_crtc *crtc);
extern int intel_enable_rc6(const struct drm_device *dev);

extern bool i915_semaphore_is_enabled(struct drm_device *dev);
int i915_reg_read_ioctl(struct drm_device *dev, void *data,
			struct drm_file *file);
int i915_get_reset_stats_ioctl(struct drm_device *dev, void *data,
			       struct drm_file *file);

void intel_notify_mmio_flip(struct intel_engine_cs *ring);

/* overlay */
extern struct intel_overlay_error_state *intel_overlay_capture_error_state(struct drm_device *dev);
extern void intel_overlay_print_error_state(struct drm_i915_error_state_buf *e,
					    struct intel_overlay_error_state *error);

extern struct intel_display_error_state *intel_display_capture_error_state(struct drm_device *dev);
extern void intel_display_print_error_state(struct drm_i915_error_state_buf *e,
					    struct drm_device *dev,
					    struct intel_display_error_state *error);

/* On SNB platform, before reading ring registers forcewake bit
 * must be set to prevent GT core from power down and stale values being
 * returned.
 */
void gen6_gt_force_wake_get(struct drm_i915_private *dev_priv, int fw_engine);
void gen6_gt_force_wake_put(struct drm_i915_private *dev_priv, int fw_engine);
void assert_force_wake_inactive(struct drm_i915_private *dev_priv);

int sandybridge_pcode_read(struct drm_i915_private *dev_priv, u32 mbox, u32 *val);
int sandybridge_pcode_write(struct drm_i915_private *dev_priv, u32 mbox, u32 val);

/* intel_sideband.c */
u32 vlv_punit_read(struct drm_i915_private *dev_priv, u8 addr);
void vlv_punit_write(struct drm_i915_private *dev_priv, u8 addr, u32 val);
u32 vlv_nc_read(struct drm_i915_private *dev_priv, u8 addr);
u32 vlv_gpio_nc_read(struct drm_i915_private *dev_priv, u32 reg);
void vlv_gpio_nc_write(struct drm_i915_private *dev_priv, u32 reg, u32 val);
u32 vlv_cck_read(struct drm_i915_private *dev_priv, u32 reg);
void vlv_cck_write(struct drm_i915_private *dev_priv, u32 reg, u32 val);
u32 vlv_ccu_read(struct drm_i915_private *dev_priv, u32 reg);
void vlv_ccu_write(struct drm_i915_private *dev_priv, u32 reg, u32 val);
u32 vlv_bunit_read(struct drm_i915_private *dev_priv, u32 reg);
void vlv_bunit_write(struct drm_i915_private *dev_priv, u32 reg, u32 val);
u32 vlv_gps_core_read(struct drm_i915_private *dev_priv, u32 reg);
void vlv_gps_core_write(struct drm_i915_private *dev_priv, u32 reg, u32 val);
u32 vlv_dpio_read(struct drm_i915_private *dev_priv, enum pipe pipe, int reg);
void vlv_dpio_write(struct drm_i915_private *dev_priv, enum pipe pipe, int reg, u32 val);
u32 intel_sbi_read(struct drm_i915_private *dev_priv, u16 reg,
		   enum intel_sbi_destination destination);
void intel_sbi_write(struct drm_i915_private *dev_priv, u16 reg, u32 value,
		     enum intel_sbi_destination destination);
u32 vlv_flisdsi_read(struct drm_i915_private *dev_priv, u32 reg);
void vlv_flisdsi_write(struct drm_i915_private *dev_priv, u32 reg, u32 val);

int vlv_gpu_freq(struct drm_i915_private *dev_priv, int val);
int vlv_freq_opcode(struct drm_i915_private *dev_priv, int val);

#define FORCEWAKE_RENDER	(1 << 0)
#define FORCEWAKE_MEDIA		(1 << 1)
#define FORCEWAKE_BLITTER	(1 << 2)
#define FORCEWAKE_ALL		(FORCEWAKE_RENDER | FORCEWAKE_MEDIA | \
					FORCEWAKE_BLITTER)


#define I915_READ8(reg)		dev_priv->uncore.funcs.mmio_readb(dev_priv, (reg), true)
#define I915_WRITE8(reg, val)	dev_priv->uncore.funcs.mmio_writeb(dev_priv, (reg), (val), true)

#define I915_READ16(reg)	dev_priv->uncore.funcs.mmio_readw(dev_priv, (reg), true)
#define I915_WRITE16(reg, val)	dev_priv->uncore.funcs.mmio_writew(dev_priv, (reg), (val), true)
#define I915_READ16_NOTRACE(reg)	dev_priv->uncore.funcs.mmio_readw(dev_priv, (reg), false)
#define I915_WRITE16_NOTRACE(reg, val)	dev_priv->uncore.funcs.mmio_writew(dev_priv, (reg), (val), false)

#define I915_READ(reg)		dev_priv->uncore.funcs.mmio_readl(dev_priv, (reg), true)
#define I915_WRITE(reg, val)	dev_priv->uncore.funcs.mmio_writel(dev_priv, (reg), (val), true)
#define I915_READ_NOTRACE(reg)		dev_priv->uncore.funcs.mmio_readl(dev_priv, (reg), false)
#define I915_WRITE_NOTRACE(reg, val)	dev_priv->uncore.funcs.mmio_writel(dev_priv, (reg), (val), false)

/* Be very careful with read/write 64-bit values. On 32-bit machines, they
 * will be implemented using 2 32-bit writes in an arbitrary order with
 * an arbitrary delay between them. This can cause the hardware to
 * act upon the intermediate value, possibly leading to corruption and
 * machine death. You have been warned.
 */
#define I915_WRITE64(reg, val)	dev_priv->uncore.funcs.mmio_writeq(dev_priv, (reg), (val), true)
#define I915_READ64(reg)	dev_priv->uncore.funcs.mmio_readq(dev_priv, (reg), true)

#define I915_READ64_2x32(lower_reg, upper_reg) ({			\
		u32 upper = I915_READ(upper_reg);			\
		u32 lower = I915_READ(lower_reg);			\
		u32 tmp = I915_READ(upper_reg);				\
		if (upper != tmp) {					\
			upper = tmp;					\
			lower = I915_READ(lower_reg);			\
			WARN_ON(I915_READ(upper_reg) != upper);		\
		}							\
		(u64)upper << 32 | lower; })

#define POSTING_READ(reg)	(void)I915_READ_NOTRACE(reg)
#define POSTING_READ16(reg)	(void)I915_READ16_NOTRACE(reg)

/* "Broadcast RGB" property */
#define INTEL_BROADCAST_RGB_AUTO 0
#define INTEL_BROADCAST_RGB_FULL 1
#define INTEL_BROADCAST_RGB_LIMITED 2

static inline uint32_t i915_vgacntrl_reg(struct drm_device *dev)
{
	if (IS_VALLEYVIEW(dev))
		return VLV_VGACNTRL;
	else if (INTEL_INFO(dev)->gen >= 5)
		return CPU_VGACNTRL;
	else
		return VGACNTRL;
}

static inline void __user *to_user_ptr(u64 address)
{
	return (void __user *)(uintptr_t)address;
}

static inline unsigned long msecs_to_jiffies_timeout(const unsigned int m)
{
	unsigned long j = msecs_to_jiffies(m);

	return min_t(unsigned long, MAX_JIFFY_OFFSET, j + 1);
}

static inline unsigned long nsecs_to_jiffies_timeout(const u64 n)
{
        return min_t(u64, MAX_JIFFY_OFFSET, nsecs_to_jiffies64(n) + 1);
}

static inline unsigned long
timespec_to_jiffies_timeout(const struct timespec *value)
{
	unsigned long j = timespec_to_jiffies(value);

	return min_t(unsigned long, MAX_JIFFY_OFFSET, j + 1);
}

/*
 * If you need to wait X milliseconds between events A and B, but event B
 * doesn't happen exactly after event A, you record the timestamp (jiffies) of
 * when event A happened, then just before event B you call this function and
 * pass the timestamp as the first argument, and X as the second argument.
 */
static inline void
wait_remaining_ms_from_jiffies(unsigned long timestamp_jiffies, int to_wait_ms)
{
	unsigned long target_jiffies, tmp_jiffies, remaining_jiffies;

	/*
	 * Don't re-read the value of "jiffies" every time since it may change
	 * behind our back and break the math.
	 */
	tmp_jiffies = jiffies;
	target_jiffies = timestamp_jiffies +
			 msecs_to_jiffies_timeout(to_wait_ms);

	if (time_after(target_jiffies, tmp_jiffies)) {
		remaining_jiffies = target_jiffies - tmp_jiffies;
		while (remaining_jiffies)
			remaining_jiffies =
			    schedule_timeout_uninterruptible(remaining_jiffies);
	}
}

#endif
{
	BUG_ON(obj->pages_pin_count == 0);
	obj->pages_pin_count--;
}

int __must_check i915_mutex_lock_interruptible(struct drm_device *dev);
int i915_gem_object_sync(struct drm_i915_gem_object *obj,
			 struct intel_engine_cs *to);
void i915_vma_move_to_active(struct i915_vma *vma,
			     struct intel_engine_cs *ring);
int i915_gem_dumb_create(struct drm_file *file_priv,
			 struct drm_device *dev,
			 struct drm_mode_create_dumb *args);
int i915_gem_mmap_gtt(struct drm_file *file_priv, struct drm_device *dev,
		      uint32_t handle, uint64_t *offset);
/**
 * Returns true if seq1 is later than seq2.
 */
static inline bool
i915_seqno_passed(uint32_t seq1, uint32_t seq2)
{
	return (int32_t)(seq1 - seq2) >= 0;
}