{
	struct rv7xx_power_info *pi;
	struct evergreen_power_info *eg_pi;
	struct ni_power_info *ni_pi;
	struct si_power_info *si_pi;
	struct atom_clock_dividers dividers;
	enum pci_bus_speed speed_cap;
	struct pci_dev *root = rdev->pdev->bus->self;
	int ret;

	si_pi = kzalloc(sizeof(struct si_power_info), GFP_KERNEL);
	if (si_pi == NULL)
		return -ENOMEM;
	rdev->pm.dpm.priv = si_pi;
	ni_pi = &si_pi->ni;
	eg_pi = &ni_pi->eg;
	pi = &eg_pi->rv7xx;

	speed_cap = pcie_get_speed_cap(root);
	if (speed_cap == PCI_SPEED_UNKNOWN) {
		si_pi->sys_pcie_mask = 0;
	} else {
		if (speed_cap == PCIE_SPEED_8_0GT)
			si_pi->sys_pcie_mask = RADEON_PCIE_SPEED_25 |
				RADEON_PCIE_SPEED_50 |
				RADEON_PCIE_SPEED_80;
		else if (speed_cap == PCIE_SPEED_5_0GT)
			si_pi->sys_pcie_mask = RADEON_PCIE_SPEED_25 |
				RADEON_PCIE_SPEED_50;
		else
			si_pi->sys_pcie_mask = RADEON_PCIE_SPEED_25;
	}
	si_pi->force_pcie_gen = RADEON_PCIE_GEN_INVALID;
	si_pi->boot_pcie_gen = si_get_current_pcie_speed(rdev);

	si_set_max_cu_value(rdev);

	rv770_get_max_vddc(rdev);
	si_get_leakage_vddc(rdev);
	si_patch_dependency_tables_based_on_leakage(rdev);

	pi->acpi_vddc = 0;
	eg_pi->acpi_vddci = 0;
	pi->min_vddc_in_table = 0;
	pi->max_vddc_in_table = 0;

	ret = r600_get_platform_caps(rdev);
	if (ret)
		return ret;

	ret = r600_parse_extended_power_table(rdev);
	if (ret)
		return ret;

	ret = si_parse_power_table(rdev);
	if (ret)
		return ret;

	rdev->pm.dpm.dyn_state.vddc_dependency_on_dispclk.entries =
		kcalloc(4,
			sizeof(struct radeon_clock_voltage_dependency_entry),
			GFP_KERNEL);
	if (!rdev->pm.dpm.dyn_state.vddc_dependency_on_dispclk.entries) {
		r600_free_extended_power_table(rdev);
		return -ENOMEM;
	}
	rdev->pm.dpm.dyn_state.vddc_dependency_on_dispclk.count = 4;
	rdev->pm.dpm.dyn_state.vddc_dependency_on_dispclk.entries[0].clk = 0;
	rdev->pm.dpm.dyn_state.vddc_dependency_on_dispclk.entries[0].v = 0;
	rdev->pm.dpm.dyn_state.vddc_dependency_on_dispclk.entries[1].clk = 36000;
	rdev->pm.dpm.dyn_state.vddc_dependency_on_dispclk.entries[1].v = 720;
	rdev->pm.dpm.dyn_state.vddc_dependency_on_dispclk.entries[2].clk = 54000;
	rdev->pm.dpm.dyn_state.vddc_dependency_on_dispclk.entries[2].v = 810;
	rdev->pm.dpm.dyn_state.vddc_dependency_on_dispclk.entries[3].clk = 72000;
	rdev->pm.dpm.dyn_state.vddc_dependency_on_dispclk.entries[3].v = 900;

	if (rdev->pm.dpm.voltage_response_time == 0)
		rdev->pm.dpm.voltage_response_time = R600_VOLTAGERESPONSETIME_DFLT;
	if (rdev->pm.dpm.backbias_response_time == 0)
		rdev->pm.dpm.backbias_response_time = R600_BACKBIASRESPONSETIME_DFLT;

	ret = radeon_atom_get_clock_dividers(rdev, COMPUTE_ENGINE_PLL_PARAM,
					     0, false, &dividers);
	if (ret)
		pi->ref_div = dividers.ref_div + 1;
	else
		pi->ref_div = R600_REFERENCEDIVIDER_DFLT;

	eg_pi->smu_uvd_hs = false;

	pi->mclk_strobe_mode_threshold = 40000;
	if (si_is_special_1gb_platform(rdev))
		pi->mclk_stutter_mode_threshold = 0;
	else
		pi->mclk_stutter_mode_threshold = pi->mclk_strobe_mode_threshold;
	pi->mclk_edc_enable_threshold = 40000;
	eg_pi->mclk_edc_wr_enable_threshold = 40000;

	ni_pi->mclk_rtt_mode_threshold = eg_pi->mclk_edc_wr_enable_threshold;

	pi->voltage_control =
		radeon_atom_is_voltage_gpio(rdev, SET_VOLTAGE_TYPE_ASIC_VDDC,
					    VOLTAGE_OBJ_GPIO_LUT);
	if (!pi->voltage_control) {
		si_pi->voltage_control_svi2 =
			radeon_atom_is_voltage_gpio(rdev, SET_VOLTAGE_TYPE_ASIC_VDDC,
						    VOLTAGE_OBJ_SVID2);
		if (si_pi->voltage_control_svi2)
			radeon_atom_get_svi2_info(rdev, SET_VOLTAGE_TYPE_ASIC_VDDC,
						  &si_pi->svd_gpio_id, &si_pi->svc_gpio_id);
	}

	pi->mvdd_control =
		radeon_atom_is_voltage_gpio(rdev, SET_VOLTAGE_TYPE_ASIC_MVDDC,
					    VOLTAGE_OBJ_GPIO_LUT);

	eg_pi->vddci_control =
		radeon_atom_is_voltage_gpio(rdev, SET_VOLTAGE_TYPE_ASIC_VDDCI,
					    VOLTAGE_OBJ_GPIO_LUT);
	if (!eg_pi->vddci_control)
		si_pi->vddci_control_svi2 =
			radeon_atom_is_voltage_gpio(rdev, SET_VOLTAGE_TYPE_ASIC_VDDCI,
						    VOLTAGE_OBJ_SVID2);

	si_pi->vddc_phase_shed_control =
		radeon_atom_is_voltage_gpio(rdev, SET_VOLTAGE_TYPE_ASIC_VDDC,
					    VOLTAGE_OBJ_PHASE_LUT);

	rv770_get_engine_memory_ss(rdev);

	pi->asi = RV770_ASI_DFLT;
	pi->pasi = CYPRESS_HASI_DFLT;
	pi->vrc = SISLANDS_VRC_DFLT;

	pi->gfx_clock_gating = true;

	eg_pi->sclk_deep_sleep = true;
	si_pi->sclk_deep_sleep_above_low = false;

	if (rdev->pm.int_thermal_type != THERMAL_TYPE_NONE)
		pi->thermal_protection = true;
	else
		pi->thermal_protection = false;

	eg_pi->dynamic_ac_timing = true;

	eg_pi->light_sleep = true;
#if defined(CONFIG_ACPI)
	eg_pi->pcie_performance_request =
		radeon_acpi_is_pcie_performance_request_supported(rdev);
#else
	eg_pi->pcie_performance_request = false;
#endif

	si_pi->sram_end = SMC_RAM_END;

	rdev->pm.dpm.dyn_state.mclk_sclk_ratio = 4;
	rdev->pm.dpm.dyn_state.sclk_mclk_delta = 15000;
	rdev->pm.dpm.dyn_state.vddc_vddci_delta = 200;
	rdev->pm.dpm.dyn_state.valid_sclk_values.count = 0;
	rdev->pm.dpm.dyn_state.valid_sclk_values.values = NULL;
	rdev->pm.dpm.dyn_state.valid_mclk_values.count = 0;
	rdev->pm.dpm.dyn_state.valid_mclk_values.values = NULL;

	si_initialize_powertune_defaults(rdev);

	/* make sure dc limits are valid */
	if ((rdev->pm.dpm.dyn_state.max_clock_voltage_on_dc.sclk == 0) ||
	    (rdev->pm.dpm.dyn_state.max_clock_voltage_on_dc.mclk == 0))
		rdev->pm.dpm.dyn_state.max_clock_voltage_on_dc =
			rdev->pm.dpm.dyn_state.max_clock_voltage_on_ac;

	si_pi->fan_ctrl_is_in_default_mode = true;

	return 0;
}
{
	struct rv7xx_power_info *pi;
	struct evergreen_power_info *eg_pi;
	struct ni_power_info *ni_pi;
	struct si_power_info *si_pi;
	struct atom_clock_dividers dividers;
	enum pci_bus_speed speed_cap;
	struct pci_dev *root = rdev->pdev->bus->self;
	int ret;

	si_pi = kzalloc(sizeof(struct si_power_info), GFP_KERNEL);
	if (si_pi == NULL)
		return -ENOMEM;
	rdev->pm.dpm.priv = si_pi;
	ni_pi = &si_pi->ni;
	eg_pi = &ni_pi->eg;
	pi = &eg_pi->rv7xx;

	speed_cap = pcie_get_speed_cap(root);
	if (speed_cap == PCI_SPEED_UNKNOWN) {
		si_pi->sys_pcie_mask = 0;
	} else {
		if (speed_cap == PCIE_SPEED_8_0GT)
			si_pi->sys_pcie_mask = RADEON_PCIE_SPEED_25 |
				RADEON_PCIE_SPEED_50 |
				RADEON_PCIE_SPEED_80;
		else if (speed_cap == PCIE_SPEED_5_0GT)
			si_pi->sys_pcie_mask = RADEON_PCIE_SPEED_25 |
				RADEON_PCIE_SPEED_50;
		else
			si_pi->sys_pcie_mask = RADEON_PCIE_SPEED_25;
	}
	si_pi->force_pcie_gen = RADEON_PCIE_GEN_INVALID;
	si_pi->boot_pcie_gen = si_get_current_pcie_speed(rdev);

	si_set_max_cu_value(rdev);

	rv770_get_max_vddc(rdev);
	si_get_leakage_vddc(rdev);
	si_patch_dependency_tables_based_on_leakage(rdev);

	pi->acpi_vddc = 0;
	eg_pi->acpi_vddci = 0;
	pi->min_vddc_in_table = 0;
	pi->max_vddc_in_table = 0;

	ret = r600_get_platform_caps(rdev);
	if (ret)
		return ret;

	ret = r600_parse_extended_power_table(rdev);
	if (ret)
		return ret;

	ret = si_parse_power_table(rdev);
	if (ret)
		return ret;

	rdev->pm.dpm.dyn_state.vddc_dependency_on_dispclk.entries =
		kcalloc(4,
			sizeof(struct radeon_clock_voltage_dependency_entry),
			GFP_KERNEL);
	if (!rdev->pm.dpm.dyn_state.vddc_dependency_on_dispclk.entries) {
		r600_free_extended_power_table(rdev);
		return -ENOMEM;
	}
	rdev->pm.dpm.dyn_state.vddc_dependency_on_dispclk.count = 4;
	rdev->pm.dpm.dyn_state.vddc_dependency_on_dispclk.entries[0].clk = 0;
	rdev->pm.dpm.dyn_state.vddc_dependency_on_dispclk.entries[0].v = 0;
	rdev->pm.dpm.dyn_state.vddc_dependency_on_dispclk.entries[1].clk = 36000;
	rdev->pm.dpm.dyn_state.vddc_dependency_on_dispclk.entries[1].v = 720;
	rdev->pm.dpm.dyn_state.vddc_dependency_on_dispclk.entries[2].clk = 54000;
	rdev->pm.dpm.dyn_state.vddc_dependency_on_dispclk.entries[2].v = 810;
	rdev->pm.dpm.dyn_state.vddc_dependency_on_dispclk.entries[3].clk = 72000;
	rdev->pm.dpm.dyn_state.vddc_dependency_on_dispclk.entries[3].v = 900;

	if (rdev->pm.dpm.voltage_response_time == 0)
		rdev->pm.dpm.voltage_response_time = R600_VOLTAGERESPONSETIME_DFLT;
	if (rdev->pm.dpm.backbias_response_time == 0)
		rdev->pm.dpm.backbias_response_time = R600_BACKBIASRESPONSETIME_DFLT;

	ret = radeon_atom_get_clock_dividers(rdev, COMPUTE_ENGINE_PLL_PARAM,
					     0, false, &dividers);
	if (ret)
		pi->ref_div = dividers.ref_div + 1;
	else
		pi->ref_div = R600_REFERENCEDIVIDER_DFLT;

	eg_pi->smu_uvd_hs = false;

	pi->mclk_strobe_mode_threshold = 40000;
	if (si_is_special_1gb_platform(rdev))
		pi->mclk_stutter_mode_threshold = 0;
	else
		pi->mclk_stutter_mode_threshold = pi->mclk_strobe_mode_threshold;
	pi->mclk_edc_enable_threshold = 40000;
	eg_pi->mclk_edc_wr_enable_threshold = 40000;

	ni_pi->mclk_rtt_mode_threshold = eg_pi->mclk_edc_wr_enable_threshold;

	pi->voltage_control =
		radeon_atom_is_voltage_gpio(rdev, SET_VOLTAGE_TYPE_ASIC_VDDC,
					    VOLTAGE_OBJ_GPIO_LUT);
	if (!pi->voltage_control) {
		si_pi->voltage_control_svi2 =
			radeon_atom_is_voltage_gpio(rdev, SET_VOLTAGE_TYPE_ASIC_VDDC,
						    VOLTAGE_OBJ_SVID2);
		if (si_pi->voltage_control_svi2)
			radeon_atom_get_svi2_info(rdev, SET_VOLTAGE_TYPE_ASIC_VDDC,
						  &si_pi->svd_gpio_id, &si_pi->svc_gpio_id);
	}

	pi->mvdd_control =
		radeon_atom_is_voltage_gpio(rdev, SET_VOLTAGE_TYPE_ASIC_MVDDC,
					    VOLTAGE_OBJ_GPIO_LUT);

	eg_pi->vddci_control =
		radeon_atom_is_voltage_gpio(rdev, SET_VOLTAGE_TYPE_ASIC_VDDCI,
					    VOLTAGE_OBJ_GPIO_LUT);
	if (!eg_pi->vddci_control)
		si_pi->vddci_control_svi2 =
			radeon_atom_is_voltage_gpio(rdev, SET_VOLTAGE_TYPE_ASIC_VDDCI,
						    VOLTAGE_OBJ_SVID2);

	si_pi->vddc_phase_shed_control =
		radeon_atom_is_voltage_gpio(rdev, SET_VOLTAGE_TYPE_ASIC_VDDC,
					    VOLTAGE_OBJ_PHASE_LUT);

	rv770_get_engine_memory_ss(rdev);

	pi->asi = RV770_ASI_DFLT;
	pi->pasi = CYPRESS_HASI_DFLT;
	pi->vrc = SISLANDS_VRC_DFLT;

	pi->gfx_clock_gating = true;

	eg_pi->sclk_deep_sleep = true;
	si_pi->sclk_deep_sleep_above_low = false;

	if (rdev->pm.int_thermal_type != THERMAL_TYPE_NONE)
		pi->thermal_protection = true;
	else
		pi->thermal_protection = false;

	eg_pi->dynamic_ac_timing = true;

	eg_pi->light_sleep = true;
#if defined(CONFIG_ACPI)
	eg_pi->pcie_performance_request =
		radeon_acpi_is_pcie_performance_request_supported(rdev);
#else
	eg_pi->pcie_performance_request = false;
#endif

	si_pi->sram_end = SMC_RAM_END;

	rdev->pm.dpm.dyn_state.mclk_sclk_ratio = 4;
	rdev->pm.dpm.dyn_state.sclk_mclk_delta = 15000;
	rdev->pm.dpm.dyn_state.vddc_vddci_delta = 200;
	rdev->pm.dpm.dyn_state.valid_sclk_values.count = 0;
	rdev->pm.dpm.dyn_state.valid_sclk_values.values = NULL;
	rdev->pm.dpm.dyn_state.valid_mclk_values.count = 0;
	rdev->pm.dpm.dyn_state.valid_mclk_values.values = NULL;

	si_initialize_powertune_defaults(rdev);

	/* make sure dc limits are valid */
	if ((rdev->pm.dpm.dyn_state.max_clock_voltage_on_dc.sclk == 0) ||
	    (rdev->pm.dpm.dyn_state.max_clock_voltage_on_dc.mclk == 0))
		rdev->pm.dpm.dyn_state.max_clock_voltage_on_dc =
			rdev->pm.dpm.dyn_state.max_clock_voltage_on_ac;

	si_pi->fan_ctrl_is_in_default_mode = true;

	return 0;
}