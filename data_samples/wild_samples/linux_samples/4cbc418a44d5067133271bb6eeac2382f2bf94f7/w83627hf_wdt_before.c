	switch (val) {
	case W83627HF_ID:
		ret = w83627hf;
		break;
	case W83627S_ID:
		ret = w83627s;
		break;
	case W83697HF_ID:
		ret = w83697hf;
		cr_wdt_timeout = W83697HF_WDT_TIMEOUT;
		cr_wdt_control = W83697HF_WDT_CONTROL;
		break;
	case W83697UG_ID:
		ret = w83697ug;
		cr_wdt_timeout = W83697HF_WDT_TIMEOUT;
		cr_wdt_control = W83697HF_WDT_CONTROL;
		break;
	case W83637HF_ID:
		ret = w83637hf;
		break;
	case W83627THF_ID:
		ret = w83627thf;
		break;
	case W83687THF_ID:
		ret = w83687thf;
		break;
	case W83627EHF_ID:
		ret = w83627ehf;
		break;
	case W83627DHG_ID:
		ret = w83627dhg;
		break;
	case W83627DHG_P_ID:
		ret = w83627dhg_p;
		break;
	case W83627UHG_ID:
		ret = w83627uhg;
		break;
	case W83667HG_ID:
		ret = w83667hg;
		break;
	case W83667HG_B_ID:
		ret = w83667hg_b;
		break;
	case NCT6775_ID:
		ret = nct6775;
		break;
	case NCT6776_ID:
		ret = nct6776;
		break;
	case NCT6779_ID:
		ret = nct6779;
		break;
	case NCT6791_ID:
		ret = nct6791;
		break;
	case NCT6792_ID:
		ret = nct6792;
		break;
	case NCT6793_ID:
		ret = nct6793;
		break;
	case NCT6795_ID:
		ret = nct6795;
		break;
	case NCT6796_ID:
		ret = nct6796;
		break;
	case NCT6102_ID:
		ret = nct6102;
		cr_wdt_timeout = NCT6102D_WDT_TIMEOUT;
		cr_wdt_control = NCT6102D_WDT_CONTROL;
		cr_wdt_csr = NCT6102D_WDT_CSR;
		break;
	case NCT6116_ID:
		ret = nct6102;
		cr_wdt_timeout = NCT6102D_WDT_TIMEOUT;
		cr_wdt_control = NCT6102D_WDT_CONTROL;
		cr_wdt_csr = NCT6102D_WDT_CSR;
		break;
	case 0xff:
		ret = -ENODEV;
		break;
	default:
		ret = -ENODEV;
		pr_err("Unsupported chip ID: 0x%02x\n", val);
		break;
	}
	superio_exit();
	return ret;
}

/*
 * On some systems, the NCT6791D comes with a companion chip and the
 * watchdog function is in this companion chip. We must use a different
 * unlocking sequence to access the companion chip.
 */
static int __init wdt_use_alt_key(const struct dmi_system_id *d)
{
	wdt_cfg_enter = 0x88;
	wdt_cfg_leave = 0xBB;

	return 0;
}

static const struct dmi_system_id wdt_dmi_table[] __initconst = {
	{
		.matches = {
			DMI_EXACT_MATCH(DMI_SYS_VENDOR, "INVES"),
			DMI_EXACT_MATCH(DMI_PRODUCT_NAME, "CTS"),
			DMI_EXACT_MATCH(DMI_BOARD_VENDOR, "INVES"),
			DMI_EXACT_MATCH(DMI_BOARD_NAME, "SHARKBAY"),
		},
		.callback = wdt_use_alt_key,
	},
	{}