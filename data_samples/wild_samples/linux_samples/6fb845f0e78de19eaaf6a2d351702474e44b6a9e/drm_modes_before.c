{
	unsigned int calc_val;

	if (mode->hsync)
		return mode->hsync;

	if (mode->htotal < 0)
		return 0;

	calc_val = (mode->clock * 1000) / mode->htotal; /* hsync in Hz */
	calc_val += 500;				/* round to 1000Hz */
	calc_val /= 1000;				/* truncate to kHz */

	return calc_val;
}
EXPORT_SYMBOL(drm_mode_hsync);

/**
 * drm_mode_vrefresh - get the vrefresh of a mode
 * @mode: mode
 *
 * Returns:
 * @modes's vrefresh rate in Hz, rounded to the nearest integer. Calculates the
 * value first if it is not yet set.
 */
int drm_mode_vrefresh(const struct drm_display_mode *mode)
{
	int refresh = 0;

	if (mode->vrefresh > 0)
		refresh = mode->vrefresh;
	else if (mode->htotal > 0 && mode->vtotal > 0) {
		unsigned int num, den;

		num = mode->clock * 1000;
		den = mode->htotal * mode->vtotal;

		if (mode->flags & DRM_MODE_FLAG_INTERLACE)
			num *= 2;
		if (mode->flags & DRM_MODE_FLAG_DBLSCAN)
			den *= 2;
		if (mode->vscan > 1)
			den *= mode->vscan;

		refresh = DIV_ROUND_CLOSEST(num, den);
	}