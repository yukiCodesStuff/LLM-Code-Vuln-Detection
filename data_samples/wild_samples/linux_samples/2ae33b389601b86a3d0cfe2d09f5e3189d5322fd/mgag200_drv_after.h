struct mga_fbdev {
	struct drm_fb_helper helper;
	struct mga_framebuffer mfb;
	void *sysram;
	int size;
	struct ttm_bo_kmap_obj mapping;
};

struct mga_crtc {
	struct drm_crtc base;
	u8 lut_r[256], lut_g[256], lut_b[256];
	int last_dpms;
	bool enabled;
};

struct mga_mode_info {
	bool mode_config_initialized;
	struct mga_crtc *crtc;
};

struct mga_encoder {
	struct drm_encoder base;
	int last_dpms;
};


struct mga_i2c_chan {
	struct i2c_adapter adapter;
	struct drm_device *dev;
	struct i2c_algo_bit_data bit;
	int data, clock;
};

struct mga_connector {
	struct drm_connector base;
	struct mga_i2c_chan *i2c;
};


struct mga_mc {
	resource_size_t			vram_size;
	resource_size_t			vram_base;
	resource_size_t			vram_window;
};

enum mga_type {
	G200_SE_A,
	G200_SE_B,
	G200_WB,
	G200_EV,
	G200_EH,
	G200_ER,
};

#define IS_G200_SE(mdev) (mdev->type == G200_SE_A || mdev->type == G200_SE_B)

struct mga_device {
	struct drm_device		*dev;
	unsigned long			flags;

	resource_size_t			rmmio_base;
	resource_size_t			rmmio_size;
	void __iomem			*rmmio;

	drm_local_map_t			*framebuffer;

	struct mga_mc			mc;
	struct mga_mode_info		mode_info;

	struct mga_fbdev *mfbdev;

	bool				suspended;
	int				num_crtc;
	enum mga_type			type;
	int				has_sdram;
	struct drm_display_mode		mode;

	int bpp_shifts[4];

	int fb_mtrr;

	struct {
		struct drm_global_reference mem_global_ref;
		struct ttm_bo_global_ref bo_global_ref;
		struct ttm_bo_device bdev;
	} ttm;

	u32 reg_1e24; /* SE model number */
};


struct mgag200_bo {
	struct ttm_buffer_object bo;
	struct ttm_placement placement;
	struct ttm_bo_kmap_obj kmap;
	struct drm_gem_object gem;
	u32 placements[3];
	int pin_count;
};
#define gem_to_mga_bo(gobj) container_of((gobj), struct mgag200_bo, gem)

static inline struct mgag200_bo *
mgag200_bo(struct ttm_buffer_object *bo)
{
	return container_of(bo, struct mgag200_bo, bo);
}
				/* mga_crtc.c */
void mga_crtc_fb_gamma_set(struct drm_crtc *crtc, u16 red, u16 green,
			     u16 blue, int regno);
void mga_crtc_fb_gamma_get(struct drm_crtc *crtc, u16 *red, u16 *green,
			     u16 *blue, int regno);

				/* mgag200_mode.c */
int mgag200_modeset_init(struct mga_device *mdev);
void mgag200_modeset_fini(struct mga_device *mdev);

				/* mga_fbdev.c */
int mgag200_fbdev_init(struct mga_device *mdev);
void mgag200_fbdev_fini(struct mga_device *mdev);

				/* mgag200_main.c */
int mgag200_framebuffer_init(struct drm_device *dev,
			     struct mga_framebuffer *mfb,
			     struct drm_mode_fb_cmd2 *mode_cmd,
			     struct drm_gem_object *obj);


int mgag200_driver_load(struct drm_device *dev, unsigned long flags);
int mgag200_driver_unload(struct drm_device *dev);
int mgag200_gem_create(struct drm_device *dev,
		   u32 size, bool iskernel,
		       struct drm_gem_object **obj);
int mgag200_gem_init_object(struct drm_gem_object *obj);
int mgag200_dumb_create(struct drm_file *file,
			struct drm_device *dev,
			struct drm_mode_create_dumb *args);
int mgag200_dumb_destroy(struct drm_file *file,
			 struct drm_device *dev,
			 uint32_t handle);
void mgag200_gem_free_object(struct drm_gem_object *obj);
int
mgag200_dumb_mmap_offset(struct drm_file *file,
			 struct drm_device *dev,
			 uint32_t handle,
			 uint64_t *offset);
				/* mga_i2c.c */
struct mga_i2c_chan *mgag200_i2c_create(struct drm_device *dev);
void mgag200_i2c_destroy(struct mga_i2c_chan *i2c);

#define DRM_FILE_PAGE_OFFSET (0x100000000ULL >> PAGE_SHIFT)
void mgag200_ttm_placement(struct mgag200_bo *bo, int domain);

int mgag200_bo_reserve(struct mgag200_bo *bo, bool no_wait);
void mgag200_bo_unreserve(struct mgag200_bo *bo);
int mgag200_bo_create(struct drm_device *dev, int size, int align,
		      uint32_t flags, struct mgag200_bo **pastbo);
int mgag200_mm_init(struct mga_device *mdev);
void mgag200_mm_fini(struct mga_device *mdev);
int mgag200_mmap(struct file *filp, struct vm_area_struct *vma);
int mgag200_bo_pin(struct mgag200_bo *bo, u32 pl_flag, u64 *gpu_addr);
int mgag200_bo_unpin(struct mgag200_bo *bo);
int mgag200_bo_push_sysram(struct mgag200_bo *bo);
#endif				/* __MGAG200_DRV_H__ */