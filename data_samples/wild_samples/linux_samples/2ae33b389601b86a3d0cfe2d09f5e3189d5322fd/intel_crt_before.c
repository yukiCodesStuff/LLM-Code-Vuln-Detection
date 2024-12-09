{
	struct drm_i915_private *dev_priv = encoder->base.dev->dev_private;
	struct intel_crt *crt = intel_encoder_to_crt(encoder);
	u32 temp;

	temp = I915_READ(crt->adpa_reg);
	temp &= ~(ADPA_HSYNC_CNTL_DISABLE | ADPA_VSYNC_CNTL_DISABLE);
	temp &= ~ADPA_DAC_ENABLE;
	I915_WRITE(crt->adpa_reg, temp);
}

static void intel_enable_crt(struct intel_encoder *encoder)
{
	struct drm_i915_private *dev_priv = encoder->base.dev->dev_private;
	struct intel_crt *crt = intel_encoder_to_crt(encoder);
	u32 temp;

	temp = I915_READ(crt->adpa_reg);
	temp |= ADPA_DAC_ENABLE;
	I915_WRITE(crt->adpa_reg, temp);
}

/* Note: The caller is required to filter out dpms modes not supported by the
 * platform. */
static void intel_crt_set_dpms(struct intel_encoder *encoder, int mode)
{
	struct drm_device *dev = encoder->base.dev;
	struct drm_i915_private *dev_priv = dev->dev_private;
	struct intel_crt *crt = intel_encoder_to_crt(encoder);
	u32 temp;

	temp = I915_READ(crt->adpa_reg);
	temp &= ~(ADPA_HSYNC_CNTL_DISABLE | ADPA_VSYNC_CNTL_DISABLE);
	temp &= ~ADPA_DAC_ENABLE;

	switch (mode) {
	case DRM_MODE_DPMS_ON:
		temp |= ADPA_DAC_ENABLE;
		break;
	case DRM_MODE_DPMS_STANDBY:
		temp |= ADPA_DAC_ENABLE | ADPA_HSYNC_CNTL_DISABLE;
		break;
	case DRM_MODE_DPMS_SUSPEND:
		temp |= ADPA_DAC_ENABLE | ADPA_VSYNC_CNTL_DISABLE;
		break;
	case DRM_MODE_DPMS_OFF:
		temp |= ADPA_HSYNC_CNTL_DISABLE | ADPA_VSYNC_CNTL_DISABLE;
		break;
	}