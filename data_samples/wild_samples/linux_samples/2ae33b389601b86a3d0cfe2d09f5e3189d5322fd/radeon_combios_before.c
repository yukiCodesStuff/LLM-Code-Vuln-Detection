		} else {
			bg = RBIOS8(dac_info + 0x2) & 0xf;
			dac = RBIOS8(dac_info + 0x3) & 0xf;
			p_dac->ps2_pdac_adj = (bg << 8) | (dac);
		}
		/* if the values are all zeros, use the table */
		if (p_dac->ps2_pdac_adj)
			found = 1;
	}

	if (!found) /* fallback to defaults */
		radeon_legacy_get_primary_dac_info_from_table(rdev, p_dac);

	return p_dac;
}

enum radeon_tv_std
radeon_combios_get_tv_info(struct radeon_device *rdev)
{
	struct drm_device *dev = rdev->ddev;
	uint16_t tv_info;
	enum radeon_tv_std tv_std = TV_STD_NTSC;

	tv_info = combios_get_table_offset(dev, COMBIOS_TV_INFO_TABLE);
	if (tv_info) {
		if (RBIOS8(tv_info + 6) == 'T') {
			switch (RBIOS8(tv_info + 7) & 0xf) {
			case 1:
				tv_std = TV_STD_NTSC;
				DRM_DEBUG_KMS("Default TV standard: NTSC\n");
				break;
			case 2:
				tv_std = TV_STD_PAL;
				DRM_DEBUG_KMS("Default TV standard: PAL\n");
				break;
			case 3:
				tv_std = TV_STD_PAL_M;
				DRM_DEBUG_KMS("Default TV standard: PAL-M\n");
				break;
			case 4:
				tv_std = TV_STD_PAL_60;
				DRM_DEBUG_KMS("Default TV standard: PAL-60\n");
				break;
			case 5:
				tv_std = TV_STD_NTSC_J;
				DRM_DEBUG_KMS("Default TV standard: NTSC-J\n");
				break;
			case 6:
				tv_std = TV_STD_SCART_PAL;
				DRM_DEBUG_KMS("Default TV standard: SCART-PAL\n");
				break;
			default:
				tv_std = TV_STD_NTSC;
				DRM_DEBUG_KMS
				    ("Unknown TV standard; defaulting to NTSC\n");
				break;
			}

			switch ((RBIOS8(tv_info + 9) >> 2) & 0x3) {
			case 0:
				DRM_DEBUG_KMS("29.498928713 MHz TV ref clk\n");
				break;
			case 1:
				DRM_DEBUG_KMS("28.636360000 MHz TV ref clk\n");
				break;
			case 2:
				DRM_DEBUG_KMS("14.318180000 MHz TV ref clk\n");
				break;
			case 3:
				DRM_DEBUG_KMS("27.000000000 MHz TV ref clk\n");
				break;
			default:
				break;
			}
		}