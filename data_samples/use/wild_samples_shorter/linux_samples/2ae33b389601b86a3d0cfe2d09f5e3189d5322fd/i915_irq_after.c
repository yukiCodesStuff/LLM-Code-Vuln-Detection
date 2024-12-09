{
	struct drm_device *dev = (struct drm_device *) arg;
	drm_i915_private_t *dev_priv = (drm_i915_private_t *) dev->dev_private;
	u32 de_iir, gt_iir, de_ier, pm_iir, sde_ier;
	irqreturn_t ret = IRQ_NONE;
	int i;

	atomic_inc(&dev_priv->irq_received);
	de_ier = I915_READ(DEIER);
	I915_WRITE(DEIER, de_ier & ~DE_MASTER_IRQ_CONTROL);

	/* Disable south interrupts. We'll only write to SDEIIR once, so further
	 * interrupts will will be stored on its back queue, and then we'll be
	 * able to process them after we restore SDEIER (as soon as we restore
	 * it, we'll get an interrupt if SDEIIR still has something to process
	 * due to its back queue). */
	sde_ier = I915_READ(SDEIER);
	I915_WRITE(SDEIER, 0);
	POSTING_READ(SDEIER);

	gt_iir = I915_READ(GTIIR);
	if (gt_iir) {
		snb_gt_irq_handler(dev, dev_priv, gt_iir);
		I915_WRITE(GTIIR, gt_iir);

	I915_WRITE(DEIER, de_ier);
	POSTING_READ(DEIER);
	I915_WRITE(SDEIER, sde_ier);
	POSTING_READ(SDEIER);

	return ret;
}

	struct drm_device *dev = (struct drm_device *) arg;
	drm_i915_private_t *dev_priv = (drm_i915_private_t *) dev->dev_private;
	int ret = IRQ_NONE;
	u32 de_iir, gt_iir, de_ier, pm_iir, sde_ier;

	atomic_inc(&dev_priv->irq_received);

	/* disable master interrupt before clearing iir  */
	I915_WRITE(DEIER, de_ier & ~DE_MASTER_IRQ_CONTROL);
	POSTING_READ(DEIER);

	/* Disable south interrupts. We'll only write to SDEIIR once, so further
	 * interrupts will will be stored on its back queue, and then we'll be
	 * able to process them after we restore SDEIER (as soon as we restore
	 * it, we'll get an interrupt if SDEIIR still has something to process
	 * due to its back queue). */
	sde_ier = I915_READ(SDEIER);
	I915_WRITE(SDEIER, 0);
	POSTING_READ(SDEIER);

	de_iir = I915_READ(DEIIR);
	gt_iir = I915_READ(GTIIR);
	pm_iir = I915_READ(GEN6_PMIIR);

done:
	I915_WRITE(DEIER, de_ier);
	POSTING_READ(DEIER);
	I915_WRITE(SDEIER, sde_ier);
	POSTING_READ(SDEIER);

	return ret;
}
