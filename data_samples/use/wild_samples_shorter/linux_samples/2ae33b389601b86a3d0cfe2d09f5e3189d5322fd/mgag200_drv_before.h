struct mga_fbdev {
	struct drm_fb_helper helper;
	struct mga_framebuffer mfb;
	struct list_head fbdev_list;
	void *sysram;
	int size;
	struct ttm_bo_kmap_obj mapping;
};