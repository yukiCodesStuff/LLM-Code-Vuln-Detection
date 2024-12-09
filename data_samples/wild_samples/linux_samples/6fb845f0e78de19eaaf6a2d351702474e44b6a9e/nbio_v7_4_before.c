{
	WREG32_FIELD15(NBIO, 0, RCC_DOORBELL_APER_EN, BIF_DOORBELL_APER_EN, enable ? 1 : 0);
}

static void nbio_v7_4_enable_doorbell_selfring_aperture(struct amdgpu_device *adev,
							bool enable)
{

}