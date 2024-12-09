struct mga_fbdev {
	struct drm_fb_helper helper;
	struct mga_framebuffer mfb;
	void *sysram;
	int size;
	struct ttm_bo_kmap_obj mapping;
};