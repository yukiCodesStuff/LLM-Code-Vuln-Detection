 *   2.27.0 - r600-SI: Add CS ioctl support for async DMA
 *   2.28.0 - r600-eg: Add MEM_WRITE packet support
 *   2.29.0 - R500 FP16 color clear registers
 *   2.30.0 - fix for FMASK texturing
 */
#define KMS_DRIVER_MAJOR	2
#define KMS_DRIVER_MINOR	30
#define KMS_DRIVER_PATCHLEVEL	0
int radeon_driver_load_kms(struct drm_device *dev, unsigned long flags);
int radeon_driver_unload_kms(struct drm_device *dev);
int radeon_driver_firstopen_kms(struct drm_device *dev);