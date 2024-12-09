	switch (adev->asic_type) {
	case CHIP_VEGA10:
		adev->asic_funcs = &soc15_asic_funcs;
		adev->cg_flags = AMD_CG_SUPPORT_GFX_MGCG |
			AMD_CG_SUPPORT_GFX_MGLS |
			AMD_CG_SUPPORT_GFX_RLC_LS |
			AMD_CG_SUPPORT_GFX_CP_LS |
			AMD_CG_SUPPORT_GFX_3D_CGCG |
			AMD_CG_SUPPORT_GFX_3D_CGLS |
			AMD_CG_SUPPORT_GFX_CGCG |
			AMD_CG_SUPPORT_GFX_CGLS |
			AMD_CG_SUPPORT_BIF_MGCG |
			AMD_CG_SUPPORT_BIF_LS |
			AMD_CG_SUPPORT_HDP_LS |
			AMD_CG_SUPPORT_DRM_MGCG |
			AMD_CG_SUPPORT_DRM_LS |
			AMD_CG_SUPPORT_ROM_MGCG |
			AMD_CG_SUPPORT_DF_MGCG |
			AMD_CG_SUPPORT_SDMA_MGCG |
			AMD_CG_SUPPORT_SDMA_LS |
			AMD_CG_SUPPORT_MC_MGCG |
			AMD_CG_SUPPORT_MC_LS;
		adev->pg_flags = 0;
		adev->external_rev_id = 0x1;
		break;
	case CHIP_VEGA12:
		adev->asic_funcs = &soc15_asic_funcs;
		adev->cg_flags = AMD_CG_SUPPORT_GFX_MGCG |
			AMD_CG_SUPPORT_GFX_MGLS |
			AMD_CG_SUPPORT_GFX_CGCG |
			AMD_CG_SUPPORT_GFX_CGLS |
			AMD_CG_SUPPORT_GFX_3D_CGCG |
			AMD_CG_SUPPORT_GFX_3D_CGLS |
			AMD_CG_SUPPORT_GFX_CP_LS |
			AMD_CG_SUPPORT_MC_LS |
			AMD_CG_SUPPORT_MC_MGCG |
			AMD_CG_SUPPORT_SDMA_MGCG |
			AMD_CG_SUPPORT_SDMA_LS |
			AMD_CG_SUPPORT_BIF_MGCG |
			AMD_CG_SUPPORT_BIF_LS |
			AMD_CG_SUPPORT_HDP_MGCG |
			AMD_CG_SUPPORT_HDP_LS |
			AMD_CG_SUPPORT_ROM_MGCG |
			AMD_CG_SUPPORT_VCE_MGCG |
			AMD_CG_SUPPORT_UVD_MGCG;
		adev->pg_flags = 0;
		adev->external_rev_id = adev->rev_id + 0x14;
		break;
	case CHIP_VEGA20:
		adev->asic_funcs = &vega20_asic_funcs;
		adev->cg_flags = AMD_CG_SUPPORT_GFX_MGCG |
			AMD_CG_SUPPORT_GFX_MGLS |
			AMD_CG_SUPPORT_GFX_CGCG |
			AMD_CG_SUPPORT_GFX_CGLS |
			AMD_CG_SUPPORT_GFX_3D_CGCG |
			AMD_CG_SUPPORT_GFX_3D_CGLS |
			AMD_CG_SUPPORT_GFX_CP_LS |
			AMD_CG_SUPPORT_MC_LS |
			AMD_CG_SUPPORT_MC_MGCG |
			AMD_CG_SUPPORT_SDMA_MGCG |
			AMD_CG_SUPPORT_SDMA_LS |
			AMD_CG_SUPPORT_BIF_MGCG |
			AMD_CG_SUPPORT_BIF_LS |
			AMD_CG_SUPPORT_HDP_MGCG |
			AMD_CG_SUPPORT_HDP_LS |
			AMD_CG_SUPPORT_ROM_MGCG |
			AMD_CG_SUPPORT_VCE_MGCG |
			AMD_CG_SUPPORT_UVD_MGCG;
		adev->pg_flags = 0;
		adev->external_rev_id = adev->rev_id + 0x28;
		break;
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
				AMD_CG_SUPPORT_GFX_CP_LS |
				AMD_CG_SUPPORT_GFX_3D_CGCG |
				AMD_CG_SUPPORT_GFX_3D_CGLS |
				AMD_CG_SUPPORT_GFX_CGCG |
				AMD_CG_SUPPORT_GFX_CGLS |
				AMD_CG_SUPPORT_BIF_LS |
				AMD_CG_SUPPORT_HDP_LS |
				AMD_CG_SUPPORT_ROM_MGCG |
				AMD_CG_SUPPORT_MC_MGCG |
				AMD_CG_SUPPORT_MC_LS |
				AMD_CG_SUPPORT_SDMA_MGCG |
				AMD_CG_SUPPORT_SDMA_LS |
				AMD_CG_SUPPORT_VCN_MGCG;

			adev->pg_flags = AMD_PG_SUPPORT_SDMA | AMD_PG_SUPPORT_VCN;
		} else if (adev->pdev->device == 0x15d8) {
			adev->cg_flags = AMD_CG_SUPPORT_GFX_MGLS |
				AMD_CG_SUPPORT_GFX_CP_LS |
				AMD_CG_SUPPORT_GFX_3D_CGCG |
				AMD_CG_SUPPORT_GFX_3D_CGLS |
				AMD_CG_SUPPORT_GFX_CGCG |
				AMD_CG_SUPPORT_GFX_CGLS |
				AMD_CG_SUPPORT_BIF_LS |
				AMD_CG_SUPPORT_HDP_LS |
				AMD_CG_SUPPORT_ROM_MGCG |
				AMD_CG_SUPPORT_MC_MGCG |
				AMD_CG_SUPPORT_MC_LS |
				AMD_CG_SUPPORT_SDMA_MGCG |
				AMD_CG_SUPPORT_SDMA_LS;

			adev->pg_flags = AMD_PG_SUPPORT_SDMA |
				AMD_PG_SUPPORT_MMHUB |
				AMD_PG_SUPPORT_VCN |
				AMD_PG_SUPPORT_VCN_DPG;
		} else {
			adev->cg_flags = AMD_CG_SUPPORT_GFX_MGCG |
				AMD_CG_SUPPORT_GFX_MGLS |
				AMD_CG_SUPPORT_GFX_RLC_LS |
				AMD_CG_SUPPORT_GFX_CP_LS |
				AMD_CG_SUPPORT_GFX_3D_CGCG |
				AMD_CG_SUPPORT_GFX_3D_CGLS |
				AMD_CG_SUPPORT_GFX_CGCG |
				AMD_CG_SUPPORT_GFX_CGLS |
				AMD_CG_SUPPORT_BIF_MGCG |
				AMD_CG_SUPPORT_BIF_LS |
				AMD_CG_SUPPORT_HDP_MGCG |
				AMD_CG_SUPPORT_HDP_LS |
				AMD_CG_SUPPORT_DRM_MGCG |
				AMD_CG_SUPPORT_DRM_LS |
				AMD_CG_SUPPORT_ROM_MGCG |
				AMD_CG_SUPPORT_MC_MGCG |
				AMD_CG_SUPPORT_MC_LS |
				AMD_CG_SUPPORT_SDMA_MGCG |
				AMD_CG_SUPPORT_SDMA_LS |
				AMD_CG_SUPPORT_VCN_MGCG;

			adev->pg_flags = AMD_PG_SUPPORT_SDMA | AMD_PG_SUPPORT_VCN;
		}

		if (adev->powerplay.pp_feature & PP_GFXOFF_MASK)
			adev->pg_flags |= AMD_PG_SUPPORT_GFX_PG |
				AMD_PG_SUPPORT_CP |
				AMD_PG_SUPPORT_RLC_SMU_HS;
		break;
	default:
		/* FIXME: not supported yet */
		return -EINVAL;
	}

	if (amdgpu_sriov_vf(adev)) {
		amdgpu_virt_init_setting(adev);
		xgpu_ai_mailbox_set_irq_funcs(adev);
	}

	return 0;
}