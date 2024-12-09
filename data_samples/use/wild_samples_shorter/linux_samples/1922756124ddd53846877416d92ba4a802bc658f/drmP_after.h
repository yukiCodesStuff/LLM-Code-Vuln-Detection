	struct platform_device *platformdev; /**< Platform device struture */

	struct drm_sg_mem *sg;	/**< Scatter gather memory */
	unsigned int num_crtcs;                  /**< Number of CRTCs on this device */
	void *dev_private;		/**< device private data */
	void *mm_private;
	struct address_space *dev_mapping;
	struct drm_sigdata sigdata;	   /**< For block_all_signals */