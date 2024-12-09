	switch (type) {
	case amd_pp_mem_clock:
		pclk_vol_table = pinfo->vdd_dep_on_mclk;
		latency_required = true;
		break;
	case amd_pp_f_clock:
		pclk_vol_table = pinfo->vdd_dep_on_fclk;
		latency_required = true;
		break;
	case amd_pp_dcf_clock:
		pclk_vol_table = pinfo->vdd_dep_on_dcefclk;
		break;
	case amd_pp_disp_clock:
		pclk_vol_table = pinfo->vdd_dep_on_dispclk;
		break;
	case amd_pp_phy_clock:
		pclk_vol_table = pinfo->vdd_dep_on_phyclk;
		break;
	case amd_pp_dpp_clock:
		pclk_vol_table = pinfo->vdd_dep_on_dppclk;
		break;
	default:
		return -EINVAL;
	}

	if (pclk_vol_table == NULL || pclk_vol_table->count == 0)
		return -EINVAL;

	clocks->num_levels = 0;
	for (i = 0; i < pclk_vol_table->count; i++) {
		clocks->data[i].clocks_in_khz = pclk_vol_table->entries[i].clk * 10;
		clocks->data[i].latency_in_us = latency_required ?
						smu10_get_mem_latency(hwmgr,
						pclk_vol_table->entries[i].clk) :
						0;
		clocks->num_levels++;
	}

	return 0;
}

static int smu10_get_clock_by_type_with_voltage(struct pp_hwmgr *hwmgr,
		enum amd_pp_clock_type type,
		struct pp_clock_levels_with_voltage *clocks)
{
	uint32_t i;
	struct smu10_hwmgr *smu10_data = (struct smu10_hwmgr *)(hwmgr->backend);
	struct smu10_clock_voltage_information *pinfo = &(smu10_data->clock_vol_info);
	struct smu10_voltage_dependency_table *pclk_vol_table = NULL;

	if (pinfo == NULL)
		return -EINVAL;

	switch (type) {
	case amd_pp_mem_clock:
		pclk_vol_table = pinfo->vdd_dep_on_mclk;
		break;
	case amd_pp_f_clock:
		pclk_vol_table = pinfo->vdd_dep_on_fclk;
		break;
	case amd_pp_dcf_clock:
		pclk_vol_table = pinfo->vdd_dep_on_dcefclk;
		break;
	case amd_pp_soc_clock:
		pclk_vol_table = pinfo->vdd_dep_on_socclk;
		break;
	case amd_pp_disp_clock:
		pclk_vol_table = pinfo->vdd_dep_on_dispclk;
		break;
	case amd_pp_phy_clock:
		pclk_vol_table = pinfo->vdd_dep_on_phyclk;
		break;
	default:
		return -EINVAL;
	}

	if (pclk_vol_table == NULL || pclk_vol_table->count == 0)
		return -EINVAL;

	clocks->num_levels = 0;
	for (i = 0; i < pclk_vol_table->count; i++) {
		clocks->data[i].clocks_in_khz = pclk_vol_table->entries[i].clk  * 10;
		clocks->data[i].voltage_in_mv = pclk_vol_table->entries[i].vol;
		clocks->num_levels++;
	}

	return 0;
}



static int smu10_get_max_high_clocks(struct pp_hwmgr *hwmgr, struct amd_pp_simple_clock_info *clocks)
{
	clocks->engine_max_clock = 80000; /* driver can't get engine clock, temp hard code to 800MHz */
	return 0;
}

static int smu10_thermal_get_temperature(struct pp_hwmgr *hwmgr)
{
	struct amdgpu_device *adev = hwmgr->adev;
	uint32_t reg_value = RREG32_SOC15(THM, 0, mmTHM_TCON_CUR_TMP);
	int cur_temp =
		(reg_value & THM_TCON_CUR_TMP__CUR_TEMP_MASK) >> THM_TCON_CUR_TMP__CUR_TEMP__SHIFT;

	if (cur_temp & THM_TCON_CUR_TMP__CUR_TEMP_RANGE_SEL_MASK)
		cur_temp = ((cur_temp / 8) - 49) * PP_TEMPERATURE_UNITS_PER_CENTIGRADES;
	else
		cur_temp = (cur_temp / 8) * PP_TEMPERATURE_UNITS_PER_CENTIGRADES;

	return cur_temp;
}

static int smu10_read_sensor(struct pp_hwmgr *hwmgr, int idx,
			  void *value, int *size)
{
	uint32_t sclk, mclk;
	int ret = 0;

	switch (idx) {
	case AMDGPU_PP_SENSOR_GFX_SCLK:
		smum_send_msg_to_smc(hwmgr, PPSMC_MSG_GetGfxclkFrequency);
		sclk = smum_get_argument(hwmgr);
			/* in units of 10KHZ */
		*((uint32_t *)value) = sclk * 100;
		*size = 4;
		break;
	case AMDGPU_PP_SENSOR_GFX_MCLK:
		smum_send_msg_to_smc(hwmgr, PPSMC_MSG_GetFclkFrequency);
		mclk = smum_get_argument(hwmgr);
			/* in units of 10KHZ */
		*((uint32_t *)value) = mclk * 100;
		*size = 4;
		break;
	case AMDGPU_PP_SENSOR_GPU_TEMP:
		*((uint32_t *)value) = smu10_thermal_get_temperature(hwmgr);
		break;
	default:
		ret = -EINVAL;
		break;
	}

	return ret;
}

static int smu10_set_watermarks_for_clocks_ranges(struct pp_hwmgr *hwmgr,
		void *clock_ranges)
{
	struct smu10_hwmgr *data = hwmgr->backend;
	struct dm_pp_wm_sets_with_clock_ranges_soc15 *wm_with_clock_ranges = clock_ranges;
	Watermarks_t *table = &(data->water_marks_table);
	int result = 0;

	smu_set_watermarks_for_clocks_ranges(table,wm_with_clock_ranges);
	smum_smc_table_manager(hwmgr, (uint8_t *)table, (uint16_t)SMU10_WMTABLE, false);
	data->water_marks_exist = true;
	return result;
}

static int smu10_smus_notify_pwe(struct pp_hwmgr *hwmgr)
{

	return smum_send_msg_to_smc(hwmgr, PPSMC_MSG_SetRccPfcPmeRestoreRegister);
}

static int smu10_powergate_mmhub(struct pp_hwmgr *hwmgr)
{
	return smum_send_msg_to_smc(hwmgr, PPSMC_MSG_PowerGateMmHub);
}

static int smu10_powergate_sdma(struct pp_hwmgr *hwmgr, bool gate)
{
	if (gate)
		return smum_send_msg_to_smc(hwmgr, PPSMC_MSG_PowerDownSdma);
	else
		return smum_send_msg_to_smc(hwmgr, PPSMC_MSG_PowerUpSdma);
}

static void smu10_powergate_vcn(struct pp_hwmgr *hwmgr, bool bgate)
{
	if (bgate) {
		amdgpu_device_ip_set_powergating_state(hwmgr->adev,
						AMD_IP_BLOCK_TYPE_VCN,
						AMD_PG_STATE_GATE);
		smum_send_msg_to_smc_with_parameter(hwmgr,
					PPSMC_MSG_PowerDownVcn, 0);
	} else {
		smum_send_msg_to_smc_with_parameter(hwmgr,
						PPSMC_MSG_PowerUpVcn, 0);
		amdgpu_device_ip_set_powergating_state(hwmgr->adev,
						AMD_IP_BLOCK_TYPE_VCN,
						AMD_PG_STATE_UNGATE);
	}