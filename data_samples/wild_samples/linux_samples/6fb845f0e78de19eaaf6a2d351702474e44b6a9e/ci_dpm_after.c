{
	int index = GetIndexIntoMasterTable(DATA, ASIC_InternalSS_Info);
	SMU7_Discrete_DpmTable  *dpm_table;
	struct radeon_gpio_rec gpio;
	u16 data_offset, size;
	u8 frev, crev;
	struct ci_power_info *pi;
	enum pci_bus_speed speed_cap = PCI_SPEED_UNKNOWN;
	struct pci_dev *root = rdev->pdev->bus->self;
	int ret;

	pi = kzalloc(sizeof(struct ci_power_info), GFP_KERNEL);
	if (pi == NULL)
		return -ENOMEM;
	rdev->pm.dpm.priv = pi;

	if (!pci_is_root_bus(rdev->pdev->bus))
		speed_cap = pcie_get_speed_cap(root);
	if (speed_cap == PCI_SPEED_UNKNOWN) {
		pi->sys_pcie_mask = 0;
	} else {
		if (speed_cap == PCIE_SPEED_8_0GT)
			pi->sys_pcie_mask = RADEON_PCIE_SPEED_25 |
				RADEON_PCIE_SPEED_50 |
				RADEON_PCIE_SPEED_80;
		else if (speed_cap == PCIE_SPEED_5_0GT)
			pi->sys_pcie_mask = RADEON_PCIE_SPEED_25 |
				RADEON_PCIE_SPEED_50;
		else
			pi->sys_pcie_mask = RADEON_PCIE_SPEED_25;
	}
	pi->force_pcie_gen = RADEON_PCIE_GEN_INVALID;

	pi->pcie_gen_performance.max = RADEON_PCIE_GEN1;
	pi->pcie_gen_performance.min = RADEON_PCIE_GEN3;
	pi->pcie_gen_powersaving.max = RADEON_PCIE_GEN1;
	pi->pcie_gen_powersaving.min = RADEON_PCIE_GEN3;

	pi->pcie_lane_performance.max = 0;
	pi->pcie_lane_performance.min = 16;
	pi->pcie_lane_powersaving.max = 0;
	pi->pcie_lane_powersaving.min = 16;

	ret = ci_get_vbios_boot_values(rdev, &pi->vbios_boot_state);
	if (ret) {
		ci_dpm_fini(rdev);
		return ret;
	}

	ret = r600_get_platform_caps(rdev);
	if (ret) {
		ci_dpm_fini(rdev);
		return ret;
	}

	ret = r600_parse_extended_power_table(rdev);
	if (ret) {
		ci_dpm_fini(rdev);
		return ret;
	}

	ret = ci_parse_power_table(rdev);
	if (ret) {
		ci_dpm_fini(rdev);
		return ret;
	}

	pi->dll_default_on = false;
	pi->sram_end = SMC_RAM_END;

	pi->activity_target[0] = CISLAND_TARGETACTIVITY_DFLT;
	pi->activity_target[1] = CISLAND_TARGETACTIVITY_DFLT;
	pi->activity_target[2] = CISLAND_TARGETACTIVITY_DFLT;
	pi->activity_target[3] = CISLAND_TARGETACTIVITY_DFLT;
	pi->activity_target[4] = CISLAND_TARGETACTIVITY_DFLT;
	pi->activity_target[5] = CISLAND_TARGETACTIVITY_DFLT;
	pi->activity_target[6] = CISLAND_TARGETACTIVITY_DFLT;
	pi->activity_target[7] = CISLAND_TARGETACTIVITY_DFLT;

	pi->mclk_activity_target = CISLAND_MCLK_TARGETACTIVITY_DFLT;

	pi->sclk_dpm_key_disabled = 0;
	pi->mclk_dpm_key_disabled = 0;
	pi->pcie_dpm_key_disabled = 0;
	pi->thermal_sclk_dpm_enabled = 0;

	/* mclk dpm is unstable on some R7 260X cards with the old mc ucode */
	if ((rdev->pdev->device == 0x6658) &&
	    (rdev->mc_fw->size == (BONAIRE_MC_UCODE_SIZE * 4))) {
		pi->mclk_dpm_key_disabled = 1;
	}

	pi->caps_sclk_ds = true;

	pi->mclk_strobe_mode_threshold = 40000;
	pi->mclk_stutter_mode_threshold = 40000;
	pi->mclk_edc_enable_threshold = 40000;
	pi->mclk_edc_wr_enable_threshold = 40000;

	ci_initialize_powertune_defaults(rdev);

	pi->caps_fps = false;

	pi->caps_sclk_throttle_low_notification = false;

	pi->caps_uvd_dpm = true;
	pi->caps_vce_dpm = true;

	ci_get_leakage_voltages(rdev);
	ci_patch_dependency_tables_with_leakage(rdev);
	ci_set_private_data_variables_based_on_pptable(rdev);

	rdev->pm.dpm.dyn_state.vddc_dependency_on_dispclk.entries =
		kcalloc(4,
			sizeof(struct radeon_clock_voltage_dependency_entry),
			GFP_KERNEL);
	if (!rdev->pm.dpm.dyn_state.vddc_dependency_on_dispclk.entries) {
		ci_dpm_fini(rdev);
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

	rdev->pm.dpm.dyn_state.mclk_sclk_ratio = 4;
	rdev->pm.dpm.dyn_state.sclk_mclk_delta = 15000;
	rdev->pm.dpm.dyn_state.vddc_vddci_delta = 200;

	rdev->pm.dpm.dyn_state.valid_sclk_values.count = 0;
	rdev->pm.dpm.dyn_state.valid_sclk_values.values = NULL;
	rdev->pm.dpm.dyn_state.valid_mclk_values.count = 0;
	rdev->pm.dpm.dyn_state.valid_mclk_values.values = NULL;

	if (rdev->family == CHIP_HAWAII) {
		pi->thermal_temp_setting.temperature_low = 94500;
		pi->thermal_temp_setting.temperature_high = 95000;
		pi->thermal_temp_setting.temperature_shutdown = 104000;
	} else {
		pi->thermal_temp_setting.temperature_low = 99500;
		pi->thermal_temp_setting.temperature_high = 100000;
		pi->thermal_temp_setting.temperature_shutdown = 104000;
	}

	pi->uvd_enabled = false;

	dpm_table = &pi->smc_state_table;

	gpio = radeon_atombios_lookup_gpio(rdev, VDDC_VRHOT_GPIO_PINID);
	if (gpio.valid) {
		dpm_table->VRHotGpio = gpio.shift;
		rdev->pm.dpm.platform_caps |= ATOM_PP_PLATFORM_CAP_REGULATOR_HOT;
	} else {
		dpm_table->VRHotGpio = CISLANDS_UNUSED_GPIO_PIN;
		rdev->pm.dpm.platform_caps &= ~ATOM_PP_PLATFORM_CAP_REGULATOR_HOT;
	}

	gpio = radeon_atombios_lookup_gpio(rdev, PP_AC_DC_SWITCH_GPIO_PINID);
	if (gpio.valid) {
		dpm_table->AcDcGpio = gpio.shift;
		rdev->pm.dpm.platform_caps |= ATOM_PP_PLATFORM_CAP_HARDWAREDC;
	} else {
		dpm_table->AcDcGpio = CISLANDS_UNUSED_GPIO_PIN;
		rdev->pm.dpm.platform_caps &= ~ATOM_PP_PLATFORM_CAP_HARDWAREDC;
	}

	gpio = radeon_atombios_lookup_gpio(rdev, VDDC_PCC_GPIO_PINID);
	if (gpio.valid) {
		u32 tmp = RREG32_SMC(CNB_PWRMGT_CNTL);

		switch (gpio.shift) {
		case 0:
			tmp &= ~GNB_SLOW_MODE_MASK;
			tmp |= GNB_SLOW_MODE(1);
			break;
		case 1:
			tmp &= ~GNB_SLOW_MODE_MASK;
			tmp |= GNB_SLOW_MODE(2);
			break;
		case 2:
			tmp |= GNB_SLOW;
			break;
		case 3:
			tmp |= FORCE_NB_PS1;
			break;
		case 4:
			tmp |= DPM_ENABLED;
			break;
		default:
			DRM_DEBUG("Invalid PCC GPIO: %u!\n", gpio.shift);
			break;
		}
		WREG32_SMC(CNB_PWRMGT_CNTL, tmp);
	}

	pi->voltage_control = CISLANDS_VOLTAGE_CONTROL_NONE;
	pi->vddci_control = CISLANDS_VOLTAGE_CONTROL_NONE;
	pi->mvdd_control = CISLANDS_VOLTAGE_CONTROL_NONE;
	if (radeon_atom_is_voltage_gpio(rdev, VOLTAGE_TYPE_VDDC, VOLTAGE_OBJ_GPIO_LUT))
		pi->voltage_control = CISLANDS_VOLTAGE_CONTROL_BY_GPIO;
	else if (radeon_atom_is_voltage_gpio(rdev, VOLTAGE_TYPE_VDDC, VOLTAGE_OBJ_SVID2))
		pi->voltage_control = CISLANDS_VOLTAGE_CONTROL_BY_SVID2;

	if (rdev->pm.dpm.platform_caps & ATOM_PP_PLATFORM_CAP_VDDCI_CONTROL) {
		if (radeon_atom_is_voltage_gpio(rdev, VOLTAGE_TYPE_VDDCI, VOLTAGE_OBJ_GPIO_LUT))
			pi->vddci_control = CISLANDS_VOLTAGE_CONTROL_BY_GPIO;
		else if (radeon_atom_is_voltage_gpio(rdev, VOLTAGE_TYPE_VDDCI, VOLTAGE_OBJ_SVID2))
			pi->vddci_control = CISLANDS_VOLTAGE_CONTROL_BY_SVID2;
		else
			rdev->pm.dpm.platform_caps &= ~ATOM_PP_PLATFORM_CAP_VDDCI_CONTROL;
	}

	if (rdev->pm.dpm.platform_caps & ATOM_PP_PLATFORM_CAP_MVDDCONTROL) {
		if (radeon_atom_is_voltage_gpio(rdev, VOLTAGE_TYPE_MVDDC, VOLTAGE_OBJ_GPIO_LUT))
			pi->mvdd_control = CISLANDS_VOLTAGE_CONTROL_BY_GPIO;
		else if (radeon_atom_is_voltage_gpio(rdev, VOLTAGE_TYPE_MVDDC, VOLTAGE_OBJ_SVID2))
			pi->mvdd_control = CISLANDS_VOLTAGE_CONTROL_BY_SVID2;
		else
			rdev->pm.dpm.platform_caps &= ~ATOM_PP_PLATFORM_CAP_MVDDCONTROL;
	}

	pi->vddc_phase_shed_control = true;

#if defined(CONFIG_ACPI)
	pi->pcie_performance_request =
		radeon_acpi_is_pcie_performance_request_supported(rdev);
#else
	pi->pcie_performance_request = false;
#endif

	if (atom_parse_data_header(rdev->mode_info.atom_context, index, &size,
				   &frev, &crev, &data_offset)) {
		pi->caps_sclk_ss_support = true;
		pi->caps_mclk_ss_support = true;
		pi->dynamic_ss = true;
	} else {
		pi->caps_sclk_ss_support = false;
		pi->caps_mclk_ss_support = false;
		pi->dynamic_ss = true;
	}

	if (rdev->pm.int_thermal_type != THERMAL_TYPE_NONE)
		pi->thermal_protection = true;
	else
		pi->thermal_protection = false;

	pi->caps_dynamic_ac_timing = true;

	pi->uvd_power_gated = false;

	/* make sure dc limits are valid */
	if ((rdev->pm.dpm.dyn_state.max_clock_voltage_on_dc.sclk == 0) ||
	    (rdev->pm.dpm.dyn_state.max_clock_voltage_on_dc.mclk == 0))
		rdev->pm.dpm.dyn_state.max_clock_voltage_on_dc =
			rdev->pm.dpm.dyn_state.max_clock_voltage_on_ac;

	pi->fan_ctrl_is_in_default_mode = true;

	return 0;
}

void ci_dpm_debugfs_print_current_performance_level(struct radeon_device *rdev,
						    struct seq_file *m)
{
	struct ci_power_info *pi = ci_get_pi(rdev);
	struct radeon_ps *rps = &pi->current_rps;
	u32 sclk = ci_get_average_sclk_freq(rdev);
	u32 mclk = ci_get_average_mclk_freq(rdev);

	seq_printf(m, "uvd    %sabled\n", pi->uvd_enabled ? "en" : "dis");
	seq_printf(m, "vce    %sabled\n", rps->vce_active ? "en" : "dis");
	seq_printf(m, "power level avg    sclk: %u mclk: %u\n",
		   sclk, mclk);
}

void ci_dpm_print_power_state(struct radeon_device *rdev,
			      struct radeon_ps *rps)
{
	struct ci_ps *ps = ci_get_ps(rps);
	struct ci_pl *pl;
	int i;

	r600_dpm_print_class_info(rps->class, rps->class2);
	r600_dpm_print_cap_info(rps->caps);
	printk("\tuvd    vclk: %d dclk: %d\n", rps->vclk, rps->dclk);
	for (i = 0; i < ps->performance_level_count; i++) {
		pl = &ps->performance_levels[i];
		printk("\t\tpower level %d    sclk: %u mclk: %u pcie gen: %u pcie lanes: %u\n",
		       i, pl->sclk, pl->mclk, pl->pcie_gen + 1, pl->pcie_lane);
	}
	r600_dpm_print_ps_status(rdev, rps);
}

u32 ci_dpm_get_current_sclk(struct radeon_device *rdev)
{
	u32 sclk = ci_get_average_sclk_freq(rdev);

	return sclk;
}

u32 ci_dpm_get_current_mclk(struct radeon_device *rdev)
{
	u32 mclk = ci_get_average_mclk_freq(rdev);

	return mclk;
}

u32 ci_dpm_get_sclk(struct radeon_device *rdev, bool low)
{
	struct ci_power_info *pi = ci_get_pi(rdev);
	struct ci_ps *requested_state = ci_get_ps(&pi->requested_rps);

	if (low)
		return requested_state->performance_levels[0].sclk;
	else
		return requested_state->performance_levels[requested_state->performance_level_count - 1].sclk;
}

u32 ci_dpm_get_mclk(struct radeon_device *rdev, bool low)
{
	struct ci_power_info *pi = ci_get_pi(rdev);
	struct ci_ps *requested_state = ci_get_ps(&pi->requested_rps);

	if (low)
		return requested_state->performance_levels[0].mclk;
	else
		return requested_state->performance_levels[requested_state->performance_level_count - 1].mclk;
}
{
	int index = GetIndexIntoMasterTable(DATA, ASIC_InternalSS_Info);
	SMU7_Discrete_DpmTable  *dpm_table;
	struct radeon_gpio_rec gpio;
	u16 data_offset, size;
	u8 frev, crev;
	struct ci_power_info *pi;
	enum pci_bus_speed speed_cap = PCI_SPEED_UNKNOWN;
	struct pci_dev *root = rdev->pdev->bus->self;
	int ret;

	pi = kzalloc(sizeof(struct ci_power_info), GFP_KERNEL);
	if (pi == NULL)
		return -ENOMEM;
	rdev->pm.dpm.priv = pi;

	if (!pci_is_root_bus(rdev->pdev->bus))
		speed_cap = pcie_get_speed_cap(root);
	if (speed_cap == PCI_SPEED_UNKNOWN) {
		pi->sys_pcie_mask = 0;
	} else {
		if (speed_cap == PCIE_SPEED_8_0GT)
			pi->sys_pcie_mask = RADEON_PCIE_SPEED_25 |
				RADEON_PCIE_SPEED_50 |
				RADEON_PCIE_SPEED_80;
		else if (speed_cap == PCIE_SPEED_5_0GT)
			pi->sys_pcie_mask = RADEON_PCIE_SPEED_25 |
				RADEON_PCIE_SPEED_50;
		else
			pi->sys_pcie_mask = RADEON_PCIE_SPEED_25;
	}
	pi->force_pcie_gen = RADEON_PCIE_GEN_INVALID;

	pi->pcie_gen_performance.max = RADEON_PCIE_GEN1;
	pi->pcie_gen_performance.min = RADEON_PCIE_GEN3;
	pi->pcie_gen_powersaving.max = RADEON_PCIE_GEN1;
	pi->pcie_gen_powersaving.min = RADEON_PCIE_GEN3;

	pi->pcie_lane_performance.max = 0;
	pi->pcie_lane_performance.min = 16;
	pi->pcie_lane_powersaving.max = 0;
	pi->pcie_lane_powersaving.min = 16;

	ret = ci_get_vbios_boot_values(rdev, &pi->vbios_boot_state);
	if (ret) {
		ci_dpm_fini(rdev);
		return ret;
	}

	ret = r600_get_platform_caps(rdev);
	if (ret) {
		ci_dpm_fini(rdev);
		return ret;
	}

	ret = r600_parse_extended_power_table(rdev);
	if (ret) {
		ci_dpm_fini(rdev);
		return ret;
	}

	ret = ci_parse_power_table(rdev);
	if (ret) {
		ci_dpm_fini(rdev);
		return ret;
	}

	pi->dll_default_on = false;
	pi->sram_end = SMC_RAM_END;

	pi->activity_target[0] = CISLAND_TARGETACTIVITY_DFLT;
	pi->activity_target[1] = CISLAND_TARGETACTIVITY_DFLT;
	pi->activity_target[2] = CISLAND_TARGETACTIVITY_DFLT;
	pi->activity_target[3] = CISLAND_TARGETACTIVITY_DFLT;
	pi->activity_target[4] = CISLAND_TARGETACTIVITY_DFLT;
	pi->activity_target[5] = CISLAND_TARGETACTIVITY_DFLT;
	pi->activity_target[6] = CISLAND_TARGETACTIVITY_DFLT;
	pi->activity_target[7] = CISLAND_TARGETACTIVITY_DFLT;

	pi->mclk_activity_target = CISLAND_MCLK_TARGETACTIVITY_DFLT;

	pi->sclk_dpm_key_disabled = 0;
	pi->mclk_dpm_key_disabled = 0;
	pi->pcie_dpm_key_disabled = 0;
	pi->thermal_sclk_dpm_enabled = 0;

	/* mclk dpm is unstable on some R7 260X cards with the old mc ucode */
	if ((rdev->pdev->device == 0x6658) &&
	    (rdev->mc_fw->size == (BONAIRE_MC_UCODE_SIZE * 4))) {
		pi->mclk_dpm_key_disabled = 1;
	}

	pi->caps_sclk_ds = true;

	pi->mclk_strobe_mode_threshold = 40000;
	pi->mclk_stutter_mode_threshold = 40000;
	pi->mclk_edc_enable_threshold = 40000;
	pi->mclk_edc_wr_enable_threshold = 40000;

	ci_initialize_powertune_defaults(rdev);

	pi->caps_fps = false;

	pi->caps_sclk_throttle_low_notification = false;

	pi->caps_uvd_dpm = true;
	pi->caps_vce_dpm = true;

	ci_get_leakage_voltages(rdev);
	ci_patch_dependency_tables_with_leakage(rdev);
	ci_set_private_data_variables_based_on_pptable(rdev);

	rdev->pm.dpm.dyn_state.vddc_dependency_on_dispclk.entries =
		kcalloc(4,
			sizeof(struct radeon_clock_voltage_dependency_entry),
			GFP_KERNEL);
	if (!rdev->pm.dpm.dyn_state.vddc_dependency_on_dispclk.entries) {
		ci_dpm_fini(rdev);
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

	rdev->pm.dpm.dyn_state.mclk_sclk_ratio = 4;
	rdev->pm.dpm.dyn_state.sclk_mclk_delta = 15000;
	rdev->pm.dpm.dyn_state.vddc_vddci_delta = 200;

	rdev->pm.dpm.dyn_state.valid_sclk_values.count = 0;
	rdev->pm.dpm.dyn_state.valid_sclk_values.values = NULL;
	rdev->pm.dpm.dyn_state.valid_mclk_values.count = 0;
	rdev->pm.dpm.dyn_state.valid_mclk_values.values = NULL;

	if (rdev->family == CHIP_HAWAII) {
		pi->thermal_temp_setting.temperature_low = 94500;
		pi->thermal_temp_setting.temperature_high = 95000;
		pi->thermal_temp_setting.temperature_shutdown = 104000;
	} else {
		pi->thermal_temp_setting.temperature_low = 99500;
		pi->thermal_temp_setting.temperature_high = 100000;
		pi->thermal_temp_setting.temperature_shutdown = 104000;
	}

	pi->uvd_enabled = false;

	dpm_table = &pi->smc_state_table;

	gpio = radeon_atombios_lookup_gpio(rdev, VDDC_VRHOT_GPIO_PINID);
	if (gpio.valid) {
		dpm_table->VRHotGpio = gpio.shift;
		rdev->pm.dpm.platform_caps |= ATOM_PP_PLATFORM_CAP_REGULATOR_HOT;
	} else {
		dpm_table->VRHotGpio = CISLANDS_UNUSED_GPIO_PIN;
		rdev->pm.dpm.platform_caps &= ~ATOM_PP_PLATFORM_CAP_REGULATOR_HOT;
	}

	gpio = radeon_atombios_lookup_gpio(rdev, PP_AC_DC_SWITCH_GPIO_PINID);
	if (gpio.valid) {
		dpm_table->AcDcGpio = gpio.shift;
		rdev->pm.dpm.platform_caps |= ATOM_PP_PLATFORM_CAP_HARDWAREDC;
	} else {
		dpm_table->AcDcGpio = CISLANDS_UNUSED_GPIO_PIN;
		rdev->pm.dpm.platform_caps &= ~ATOM_PP_PLATFORM_CAP_HARDWAREDC;
	}

	gpio = radeon_atombios_lookup_gpio(rdev, VDDC_PCC_GPIO_PINID);
	if (gpio.valid) {
		u32 tmp = RREG32_SMC(CNB_PWRMGT_CNTL);

		switch (gpio.shift) {
		case 0:
			tmp &= ~GNB_SLOW_MODE_MASK;
			tmp |= GNB_SLOW_MODE(1);
			break;
		case 1:
			tmp &= ~GNB_SLOW_MODE_MASK;
			tmp |= GNB_SLOW_MODE(2);
			break;
		case 2:
			tmp |= GNB_SLOW;
			break;
		case 3:
			tmp |= FORCE_NB_PS1;
			break;
		case 4:
			tmp |= DPM_ENABLED;
			break;
		default:
			DRM_DEBUG("Invalid PCC GPIO: %u!\n", gpio.shift);
			break;
		}
		WREG32_SMC(CNB_PWRMGT_CNTL, tmp);
	}

	pi->voltage_control = CISLANDS_VOLTAGE_CONTROL_NONE;
	pi->vddci_control = CISLANDS_VOLTAGE_CONTROL_NONE;
	pi->mvdd_control = CISLANDS_VOLTAGE_CONTROL_NONE;
	if (radeon_atom_is_voltage_gpio(rdev, VOLTAGE_TYPE_VDDC, VOLTAGE_OBJ_GPIO_LUT))
		pi->voltage_control = CISLANDS_VOLTAGE_CONTROL_BY_GPIO;
	else if (radeon_atom_is_voltage_gpio(rdev, VOLTAGE_TYPE_VDDC, VOLTAGE_OBJ_SVID2))
		pi->voltage_control = CISLANDS_VOLTAGE_CONTROL_BY_SVID2;

	if (rdev->pm.dpm.platform_caps & ATOM_PP_PLATFORM_CAP_VDDCI_CONTROL) {
		if (radeon_atom_is_voltage_gpio(rdev, VOLTAGE_TYPE_VDDCI, VOLTAGE_OBJ_GPIO_LUT))
			pi->vddci_control = CISLANDS_VOLTAGE_CONTROL_BY_GPIO;
		else if (radeon_atom_is_voltage_gpio(rdev, VOLTAGE_TYPE_VDDCI, VOLTAGE_OBJ_SVID2))
			pi->vddci_control = CISLANDS_VOLTAGE_CONTROL_BY_SVID2;
		else
			rdev->pm.dpm.platform_caps &= ~ATOM_PP_PLATFORM_CAP_VDDCI_CONTROL;
	}

	if (rdev->pm.dpm.platform_caps & ATOM_PP_PLATFORM_CAP_MVDDCONTROL) {
		if (radeon_atom_is_voltage_gpio(rdev, VOLTAGE_TYPE_MVDDC, VOLTAGE_OBJ_GPIO_LUT))
			pi->mvdd_control = CISLANDS_VOLTAGE_CONTROL_BY_GPIO;
		else if (radeon_atom_is_voltage_gpio(rdev, VOLTAGE_TYPE_MVDDC, VOLTAGE_OBJ_SVID2))
			pi->mvdd_control = CISLANDS_VOLTAGE_CONTROL_BY_SVID2;
		else
			rdev->pm.dpm.platform_caps &= ~ATOM_PP_PLATFORM_CAP_MVDDCONTROL;
	}

	pi->vddc_phase_shed_control = true;

#if defined(CONFIG_ACPI)
	pi->pcie_performance_request =
		radeon_acpi_is_pcie_performance_request_supported(rdev);
#else
	pi->pcie_performance_request = false;
#endif

	if (atom_parse_data_header(rdev->mode_info.atom_context, index, &size,
				   &frev, &crev, &data_offset)) {
		pi->caps_sclk_ss_support = true;
		pi->caps_mclk_ss_support = true;
		pi->dynamic_ss = true;
	} else {
		pi->caps_sclk_ss_support = false;
		pi->caps_mclk_ss_support = false;
		pi->dynamic_ss = true;
	}

	if (rdev->pm.int_thermal_type != THERMAL_TYPE_NONE)
		pi->thermal_protection = true;
	else
		pi->thermal_protection = false;

	pi->caps_dynamic_ac_timing = true;

	pi->uvd_power_gated = false;

	/* make sure dc limits are valid */
	if ((rdev->pm.dpm.dyn_state.max_clock_voltage_on_dc.sclk == 0) ||
	    (rdev->pm.dpm.dyn_state.max_clock_voltage_on_dc.mclk == 0))
		rdev->pm.dpm.dyn_state.max_clock_voltage_on_dc =
			rdev->pm.dpm.dyn_state.max_clock_voltage_on_ac;

	pi->fan_ctrl_is_in_default_mode = true;

	return 0;
}

void ci_dpm_debugfs_print_current_performance_level(struct radeon_device *rdev,
						    struct seq_file *m)
{
	struct ci_power_info *pi = ci_get_pi(rdev);
	struct radeon_ps *rps = &pi->current_rps;
	u32 sclk = ci_get_average_sclk_freq(rdev);
	u32 mclk = ci_get_average_mclk_freq(rdev);

	seq_printf(m, "uvd    %sabled\n", pi->uvd_enabled ? "en" : "dis");
	seq_printf(m, "vce    %sabled\n", rps->vce_active ? "en" : "dis");
	seq_printf(m, "power level avg    sclk: %u mclk: %u\n",
		   sclk, mclk);
}

void ci_dpm_print_power_state(struct radeon_device *rdev,
			      struct radeon_ps *rps)
{
	struct ci_ps *ps = ci_get_ps(rps);
	struct ci_pl *pl;
	int i;

	r600_dpm_print_class_info(rps->class, rps->class2);
	r600_dpm_print_cap_info(rps->caps);
	printk("\tuvd    vclk: %d dclk: %d\n", rps->vclk, rps->dclk);
	for (i = 0; i < ps->performance_level_count; i++) {
		pl = &ps->performance_levels[i];
		printk("\t\tpower level %d    sclk: %u mclk: %u pcie gen: %u pcie lanes: %u\n",
		       i, pl->sclk, pl->mclk, pl->pcie_gen + 1, pl->pcie_lane);
	}
	r600_dpm_print_ps_status(rdev, rps);
}

u32 ci_dpm_get_current_sclk(struct radeon_device *rdev)
{
	u32 sclk = ci_get_average_sclk_freq(rdev);

	return sclk;
}

u32 ci_dpm_get_current_mclk(struct radeon_device *rdev)
{
	u32 mclk = ci_get_average_mclk_freq(rdev);

	return mclk;
}

u32 ci_dpm_get_sclk(struct radeon_device *rdev, bool low)
{
	struct ci_power_info *pi = ci_get_pi(rdev);
	struct ci_ps *requested_state = ci_get_ps(&pi->requested_rps);

	if (low)
		return requested_state->performance_levels[0].sclk;
	else
		return requested_state->performance_levels[requested_state->performance_level_count - 1].sclk;
}

u32 ci_dpm_get_mclk(struct radeon_device *rdev, bool low)
{
	struct ci_power_info *pi = ci_get_pi(rdev);
	struct ci_ps *requested_state = ci_get_ps(&pi->requested_rps);

	if (low)
		return requested_state->performance_levels[0].mclk;
	else
		return requested_state->performance_levels[requested_state->performance_level_count - 1].mclk;
}