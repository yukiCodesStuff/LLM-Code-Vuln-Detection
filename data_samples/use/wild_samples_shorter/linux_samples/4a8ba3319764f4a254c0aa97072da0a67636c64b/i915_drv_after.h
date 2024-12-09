	 */
	struct workqueue_struct *dp_wq;

	/* Abstract the submission mechanism (legacy ringbuffer or execlists) away */
	struct {
		int (*do_execbuf)(struct drm_device *dev, struct drm_file *file,
				  struct intel_engine_cs *ring,
int i915_gem_dumb_create(struct drm_file *file_priv,
			 struct drm_device *dev,
			 struct drm_mode_create_dumb *args);
int i915_gem_mmap_gtt(struct drm_file *file_priv, struct drm_device *dev,
		      uint32_t handle, uint64_t *offset);
/**
 * Returns true if seq1 is later than seq2.
 */
static inline bool