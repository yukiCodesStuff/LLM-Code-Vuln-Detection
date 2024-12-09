	case CHIP_RAVEN:
		adev->asic_funcs = &soc15_asic_funcs;
		if (adev->rev_id >= 0x8)
			adev->external_rev_id = adev->rev_id + 0x79;
		else if (adev->pdev->device == 0x15d8)
			adev->external_rev_id = adev->rev_id + 0x41;
		else if (adev->rev_id == 1)
			adev->external_rev_id = adev->rev_id + 0x20;
		else
			adev->external_rev_id = adev->rev_id + 0x01;

		if (adev->rev_id >= 0x8) {
			adev->cg_flags = AMD_CG_SUPPORT_GFX_MGCG |
				AMD_CG_SUPPORT_GFX_MGLS |