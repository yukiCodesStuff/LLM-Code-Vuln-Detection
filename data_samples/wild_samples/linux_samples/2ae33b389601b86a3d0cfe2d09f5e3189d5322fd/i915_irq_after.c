{
	struct drm_device *dev = (struct drm_device *) arg;
	drm_i915_private_t *dev_priv = (drm_i915_private_t *) dev->dev_private;
	u32 de_iir, gt_iir, de_ier, pm_iir, sde_ier;
	irqreturn_t ret = IRQ_NONE;
	int i;

	atomic_inc(&dev_priv->irq_received);

	/* disable master interrupt before clearing iir  */
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
		ret = IRQ_HANDLED;
	}
{
	struct drm_device *dev = (struct drm_device *) arg;
	drm_i915_private_t *dev_priv = (drm_i915_private_t *) dev->dev_private;
	u32 de_iir, gt_iir, de_ier, pm_iir, sde_ier;
	irqreturn_t ret = IRQ_NONE;
	int i;

	atomic_inc(&dev_priv->irq_received);

	/* disable master interrupt before clearing iir  */
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
		ret = IRQ_HANDLED;
	}
	if (pm_iir) {
		if (pm_iir & GEN6_PM_DEFERRED_EVENTS)
			gen6_queue_rps_work(dev_priv, pm_iir);
		I915_WRITE(GEN6_PMIIR, pm_iir);
		ret = IRQ_HANDLED;
	}

	I915_WRITE(DEIER, de_ier);
	POSTING_READ(DEIER);
	I915_WRITE(SDEIER, sde_ier);
	POSTING_READ(SDEIER);

	return ret;
}

static void ilk_gt_irq_handler(struct drm_device *dev,
			       struct drm_i915_private *dev_priv,
			       u32 gt_iir)
{
	if (gt_iir & (GT_USER_INTERRUPT | GT_PIPE_NOTIFY))
		notify_ring(dev, &dev_priv->ring[RCS]);
	if (gt_iir & GT_BSD_USER_INTERRUPT)
		notify_ring(dev, &dev_priv->ring[VCS]);
}

static irqreturn_t ironlake_irq_handler(int irq, void *arg)
{
	struct drm_device *dev = (struct drm_device *) arg;
	drm_i915_private_t *dev_priv = (drm_i915_private_t *) dev->dev_private;
	int ret = IRQ_NONE;
	u32 de_iir, gt_iir, de_ier, pm_iir, sde_ier;

	atomic_inc(&dev_priv->irq_received);

	/* disable master interrupt before clearing iir  */
	de_ier = I915_READ(DEIER);
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

	if (de_iir == 0 && gt_iir == 0 && (!IS_GEN6(dev) || pm_iir == 0))
		goto done;

	ret = IRQ_HANDLED;

	if (IS_GEN5(dev))
		ilk_gt_irq_handler(dev, dev_priv, gt_iir);
	else
		snb_gt_irq_handler(dev, dev_priv, gt_iir);

	if (de_iir & DE_AUX_CHANNEL_A)
		dp_aux_irq_handler(dev);

	if (de_iir & DE_GSE)
		intel_opregion_gse_intr(dev);

	if (de_iir & DE_PIPEA_VBLANK)
		drm_handle_vblank(dev, 0);

	if (de_iir & DE_PIPEB_VBLANK)
		drm_handle_vblank(dev, 1);

	if (de_iir & DE_PLANEA_FLIP_DONE) {
		intel_prepare_page_flip(dev, 0);
		intel_finish_page_flip_plane(dev, 0);
	}
{
	struct drm_device *dev = (struct drm_device *) arg;
	drm_i915_private_t *dev_priv = (drm_i915_private_t *) dev->dev_private;
	int ret = IRQ_NONE;
	u32 de_iir, gt_iir, de_ier, pm_iir, sde_ier;

	atomic_inc(&dev_priv->irq_received);

	/* disable master interrupt before clearing iir  */
	de_ier = I915_READ(DEIER);
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

	if (de_iir == 0 && gt_iir == 0 && (!IS_GEN6(dev) || pm_iir == 0))
		goto done;

	ret = IRQ_HANDLED;

	if (IS_GEN5(dev))
		ilk_gt_irq_handler(dev, dev_priv, gt_iir);
	else
		snb_gt_irq_handler(dev, dev_priv, gt_iir);

	if (de_iir & DE_AUX_CHANNEL_A)
		dp_aux_irq_handler(dev);

	if (de_iir & DE_GSE)
		intel_opregion_gse_intr(dev);

	if (de_iir & DE_PIPEA_VBLANK)
		drm_handle_vblank(dev, 0);

	if (de_iir & DE_PIPEB_VBLANK)
		drm_handle_vblank(dev, 1);

	if (de_iir & DE_PLANEA_FLIP_DONE) {
		intel_prepare_page_flip(dev, 0);
		intel_finish_page_flip_plane(dev, 0);
	}
{
	struct drm_device *dev = (struct drm_device *) arg;
	drm_i915_private_t *dev_priv = (drm_i915_private_t *) dev->dev_private;
	int ret = IRQ_NONE;
	u32 de_iir, gt_iir, de_ier, pm_iir, sde_ier;

	atomic_inc(&dev_priv->irq_received);

	/* disable master interrupt before clearing iir  */
	de_ier = I915_READ(DEIER);
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

	if (de_iir == 0 && gt_iir == 0 && (!IS_GEN6(dev) || pm_iir == 0))
		goto done;

	ret = IRQ_HANDLED;

	if (IS_GEN5(dev))
		ilk_gt_irq_handler(dev, dev_priv, gt_iir);
	else
		snb_gt_irq_handler(dev, dev_priv, gt_iir);

	if (de_iir & DE_AUX_CHANNEL_A)
		dp_aux_irq_handler(dev);

	if (de_iir & DE_GSE)
		intel_opregion_gse_intr(dev);

	if (de_iir & DE_PIPEA_VBLANK)
		drm_handle_vblank(dev, 0);

	if (de_iir & DE_PIPEB_VBLANK)
		drm_handle_vblank(dev, 1);

	if (de_iir & DE_PLANEA_FLIP_DONE) {
		intel_prepare_page_flip(dev, 0);
		intel_finish_page_flip_plane(dev, 0);
	}
	if (de_iir & DE_PCH_EVENT) {
		u32 pch_iir = I915_READ(SDEIIR);

		if (HAS_PCH_CPT(dev))
			cpt_irq_handler(dev, pch_iir);
		else
			ibx_irq_handler(dev, pch_iir);

		/* should clear PCH hotplug event before clear CPU irq */
		I915_WRITE(SDEIIR, pch_iir);
	}

	if (IS_GEN5(dev) &&  de_iir & DE_PCU_EVENT)
		ironlake_handle_rps_change(dev);

	if (IS_GEN6(dev) && pm_iir & GEN6_PM_DEFERRED_EVENTS)
		gen6_queue_rps_work(dev_priv, pm_iir);

	I915_WRITE(GTIIR, gt_iir);
	I915_WRITE(DEIIR, de_iir);
	I915_WRITE(GEN6_PMIIR, pm_iir);

done:
	I915_WRITE(DEIER, de_ier);
	POSTING_READ(DEIER);
	I915_WRITE(SDEIER, sde_ier);
	POSTING_READ(SDEIER);

	return ret;
}

/**
 * i915_error_work_func - do process context error handling work
 * @work: work struct
 *
 * Fire an error uevent so userspace can see that a hang or error
 * was detected.
 */
static void i915_error_work_func(struct work_struct *work)
{
	struct i915_gpu_error *error = container_of(work, struct i915_gpu_error,
						    work);
	drm_i915_private_t *dev_priv = container_of(error, drm_i915_private_t,
						    gpu_error);
	struct drm_device *dev = dev_priv->dev;
	struct intel_ring_buffer *ring;
	char *error_event[] = { "ERROR=1", NULL };
	char *reset_event[] = { "RESET=1", NULL };
	char *reset_done_event[] = { "ERROR=0", NULL };
	int i, ret;

	kobject_uevent_env(&dev->primary->kdev.kobj, KOBJ_CHANGE, error_event);

	/*
	 * Note that there's only one work item which does gpu resets, so we
	 * need not worry about concurrent gpu resets potentially incrementing
	 * error->reset_counter twice. We only need to take care of another
	 * racing irq/hangcheck declaring the gpu dead for a second time. A
	 * quick check for that is good enough: schedule_work ensures the
	 * correct ordering between hang detection and this work item, and since
	 * the reset in-progress bit is only ever set by code outside of this
	 * work we don't need to worry about any other races.
	 */
	if (i915_reset_in_progress(error) && !i915_terminally_wedged(error)) {
		DRM_DEBUG_DRIVER("resetting chip\n");
		kobject_uevent_env(&dev->primary->kdev.kobj, KOBJ_CHANGE,
				   reset_event);

		ret = i915_reset(dev);

		if (ret == 0) {
			/*
			 * After all the gem state is reset, increment the reset
			 * counter and wake up everyone waiting for the reset to
			 * complete.
			 *
			 * Since unlock operations are a one-sided barrier only,
			 * we need to insert a barrier here to order any seqno
			 * updates before
			 * the counter increment.
			 */
			smp_mb__before_atomic_inc();
			atomic_inc(&dev_priv->gpu_error.reset_counter);

			kobject_uevent_env(&dev->primary->kdev.kobj,
					   KOBJ_CHANGE, reset_done_event);
		} else {
			atomic_set(&error->reset_counter, I915_WEDGED);
		}

		for_each_ring(ring, dev_priv, i)
			wake_up_all(&ring->irq_queue);

		wake_up_all(&dev_priv->gpu_error.reset_queue);
	}
}