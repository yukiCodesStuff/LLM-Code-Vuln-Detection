	if (!hsp_div[hsp_sel]) {
		pr_err("i.MX35 clk: illegal hsp clk selection 0x%x\n", hsp_sel);
		hsp_sel = 0;
	}

	clk[hsp] = imx_clk_fixed_factor("hsp", "arm", 1, hsp_div[hsp_sel]);

	clk[ahb] = imx_clk_fixed_factor("ahb", "arm", 1, aad->ahb);
	clk[ipg] = imx_clk_fixed_factor("ipg", "ahb", 1, 2);

	clk[arm_per_div] = imx_clk_divider("arm_per_div", "arm", base + MX35_CCM_PDR4, 16, 6);
	clk[ahb_per_div] = imx_clk_divider("ahb_per_div", "ahb", base + MXC_CCM_PDR0, 12, 3);
	clk[ipg_per] = imx_clk_mux("ipg_per", base + MXC_CCM_PDR0, 26, 1, ipg_per_sel, ARRAY_SIZE(ipg_per_sel));

	clk[uart_sel] = imx_clk_mux("uart_sel", base + MX35_CCM_PDR3, 14, 1, std_sel, ARRAY_SIZE(std_sel));
	clk[uart_div] = imx_clk_divider("uart_div", "uart_sel", base + MX35_CCM_PDR4, 10, 6);

	clk[esdhc_sel] = imx_clk_mux("esdhc_sel", base + MX35_CCM_PDR4, 9, 1, std_sel, ARRAY_SIZE(std_sel));
	clk[esdhc1_div] = imx_clk_divider("esdhc1_div", "esdhc_sel", base + MX35_CCM_PDR3, 0, 6);
	clk[esdhc2_div] = imx_clk_divider("esdhc2_div", "esdhc_sel", base + MX35_CCM_PDR3, 8, 6);
	clk[esdhc3_div] = imx_clk_divider("esdhc3_div", "esdhc_sel", base + MX35_CCM_PDR3, 16, 6);

	clk[spdif_sel] = imx_clk_mux("spdif_sel", base + MX35_CCM_PDR3, 22, 1, std_sel, ARRAY_SIZE(std_sel));
	clk[spdif_div_pre] = imx_clk_divider("spdif_div_pre", "spdif_sel", base + MX35_CCM_PDR3, 29, 3); /* divide by 1 not allowed */ 
	clk[spdif_div_post] = imx_clk_divider("spdif_div_post", "spdif_div_pre", base + MX35_CCM_PDR3, 23, 6);

	clk[ssi_sel] = imx_clk_mux("ssi_sel", base + MX35_CCM_PDR2, 6, 1, std_sel, ARRAY_SIZE(std_sel));
	clk[ssi1_div_pre] = imx_clk_divider("ssi1_div_pre", "ssi_sel", base + MX35_CCM_PDR2, 24, 3);
	clk[ssi1_div_post] = imx_clk_divider("ssi1_div_post", "ssi1_div_pre", base + MX35_CCM_PDR2, 0, 6);
	clk[ssi2_div_pre] = imx_clk_divider("ssi2_div_pre", "ssi_sel", base + MX35_CCM_PDR2, 27, 3);
	clk[ssi2_div_post] = imx_clk_divider("ssi2_div_post", "ssi2_div_pre", base + MX35_CCM_PDR2, 8, 6);

	clk[usb_sel] = imx_clk_mux("usb_sel", base + MX35_CCM_PDR4, 9, 1, std_sel, ARRAY_SIZE(std_sel));
	clk[usb_div] = imx_clk_divider("usb_div", "usb_sel", base + MX35_CCM_PDR4, 22, 6);

	clk[nfc_div] = imx_clk_divider("nfc_div", "ahb", base + MX35_CCM_PDR4, 28, 4);

	clk[csi_sel] = imx_clk_mux("csi_sel", base + MX35_CCM_PDR2, 7, 1, std_sel, ARRAY_SIZE(std_sel));
	clk[csi_div] = imx_clk_divider("csi_div", "csi_sel", base + MX35_CCM_PDR2, 16, 6);

	clk[asrc_gate] = imx_clk_gate2("asrc_gate", "ipg", base + MX35_CCM_CGR0,  0);
	clk[pata_gate] = imx_clk_gate2("pata_gate", "ipg", base + MX35_CCM_CGR0,  2);
	clk[audmux_gate] = imx_clk_gate2("audmux_gate", "ipg", base + MX35_CCM_CGR0,  4);
	clk[can1_gate] = imx_clk_gate2("can1_gate", "ipg", base + MX35_CCM_CGR0,  6);
	clk[can2_gate] = imx_clk_gate2("can2_gate", "ipg", base + MX35_CCM_CGR0,  8);
	clk[cspi1_gate] = imx_clk_gate2("cspi1_gate", "ipg", base + MX35_CCM_CGR0, 10);
	clk[cspi2_gate] = imx_clk_gate2("cspi2_gate", "ipg", base + MX35_CCM_CGR0, 12);
	clk[ect_gate] = imx_clk_gate2("ect_gate", "ipg", base + MX35_CCM_CGR0, 14);
	clk[edio_gate] = imx_clk_gate2("edio_gate",   "ipg", base + MX35_CCM_CGR0, 16);
	clk[emi_gate] = imx_clk_gate2("emi_gate", "ipg", base + MX35_CCM_CGR0, 18);
	clk[epit1_gate] = imx_clk_gate2("epit1_gate", "ipg", base + MX35_CCM_CGR0, 20);
	clk[epit2_gate] = imx_clk_gate2("epit2_gate", "ipg", base + MX35_CCM_CGR0, 22);
	clk[esai_gate] = imx_clk_gate2("esai_gate",   "ipg", base + MX35_CCM_CGR0, 24);
	clk[esdhc1_gate] = imx_clk_gate2("esdhc1_gate", "esdhc1_div", base + MX35_CCM_CGR0, 26);
	clk[esdhc2_gate] = imx_clk_gate2("esdhc2_gate", "esdhc2_div", base + MX35_CCM_CGR0, 28);
	clk[esdhc3_gate] = imx_clk_gate2("esdhc3_gate", "esdhc3_div", base + MX35_CCM_CGR0, 30);

	clk[fec_gate] = imx_clk_gate2("fec_gate", "ipg", base + MX35_CCM_CGR1,  0);
	clk[gpio1_gate] = imx_clk_gate2("gpio1_gate", "ipg", base + MX35_CCM_CGR1,  2);
	clk[gpio2_gate] = imx_clk_gate2("gpio2_gate", "ipg", base + MX35_CCM_CGR1,  4);
	clk[gpio3_gate] = imx_clk_gate2("gpio3_gate", "ipg", base + MX35_CCM_CGR1,  6);
	clk[gpt_gate] = imx_clk_gate2("gpt_gate", "ipg", base + MX35_CCM_CGR1,  8);
	clk[i2c1_gate] = imx_clk_gate2("i2c1_gate", "ipg_per", base + MX35_CCM_CGR1, 10);
	clk[i2c2_gate] = imx_clk_gate2("i2c2_gate", "ipg_per", base + MX35_CCM_CGR1, 12);
	clk[i2c3_gate] = imx_clk_gate2("i2c3_gate", "ipg_per", base + MX35_CCM_CGR1, 14);
	clk[iomuxc_gate] = imx_clk_gate2("iomuxc_gate", "ipg", base + MX35_CCM_CGR1, 16);
	clk[ipu_gate] = imx_clk_gate2("ipu_gate", "hsp", base + MX35_CCM_CGR1, 18);
	clk[kpp_gate] = imx_clk_gate2("kpp_gate", "ipg", base + MX35_CCM_CGR1, 20);
	clk[mlb_gate] = imx_clk_gate2("mlb_gate", "ahb", base + MX35_CCM_CGR1, 22);
	clk[mshc_gate] = imx_clk_gate2("mshc_gate", "dummy", base + MX35_CCM_CGR1, 24);
	clk[owire_gate] = imx_clk_gate2("owire_gate", "ipg_per", base + MX35_CCM_CGR1, 26);
	clk[pwm_gate] = imx_clk_gate2("pwm_gate", "ipg_per", base + MX35_CCM_CGR1, 28);
	clk[rngc_gate] = imx_clk_gate2("rngc_gate", "ipg", base + MX35_CCM_CGR1, 30);

	clk[rtc_gate] = imx_clk_gate2("rtc_gate", "ipg", base + MX35_CCM_CGR2,  0);
	clk[rtic_gate] = imx_clk_gate2("rtic_gate", "ahb", base + MX35_CCM_CGR2,  2);
	clk[scc_gate] = imx_clk_gate2("scc_gate", "ipg", base + MX35_CCM_CGR2,  4);
	clk[sdma_gate] = imx_clk_gate2("sdma_gate", "ahb", base + MX35_CCM_CGR2,  6);
	clk[spba_gate] = imx_clk_gate2("spba_gate", "ipg", base + MX35_CCM_CGR2,  8);
	clk[spdif_gate] = imx_clk_gate2("spdif_gate", "spdif_div_post", base + MX35_CCM_CGR2, 10);
	clk[ssi1_gate] = imx_clk_gate2("ssi1_gate", "ssi1_div_post", base + MX35_CCM_CGR2, 12);
	clk[ssi2_gate] = imx_clk_gate2("ssi2_gate", "ssi2_div_post", base + MX35_CCM_CGR2, 14);
	clk[uart1_gate] = imx_clk_gate2("uart1_gate", "uart_div", base + MX35_CCM_CGR2, 16);
	clk[uart2_gate] = imx_clk_gate2("uart2_gate", "uart_div", base + MX35_CCM_CGR2, 18);
	clk[uart3_gate] = imx_clk_gate2("uart3_gate", "uart_div", base + MX35_CCM_CGR2, 20);
	clk[usbotg_gate] = imx_clk_gate2("usbotg_gate", "ahb", base + MX35_CCM_CGR2, 22);
	clk[wdog_gate] = imx_clk_gate2("wdog_gate", "ipg", base + MX35_CCM_CGR2, 24);
	clk[max_gate] = imx_clk_gate2("max_gate", "dummy", base + MX35_CCM_CGR2, 26);
	clk[admux_gate] = imx_clk_gate2("admux_gate", "ipg", base + MX35_CCM_CGR2, 30);

	clk[csi_gate] = imx_clk_gate2("csi_gate", "csi_div", base + MX35_CCM_CGR3,  0);
	clk[iim_gate] = imx_clk_gate2("iim_gate", "ipg", base + MX35_CCM_CGR3,  2);
	clk[gpu2d_gate] = imx_clk_gate2("gpu2d_gate", "ahb", base + MX35_CCM_CGR3,  4);

	for (i = 0; i < ARRAY_SIZE(clk); i++)
		if (IS_ERR(clk[i]))
			pr_err("i.MX35 clk %d: register failed with %ld\n",
				i, PTR_ERR(clk[i]));

	clk_register_clkdev(clk[pata_gate], NULL, "pata_imx");
	clk_register_clkdev(clk[can1_gate], NULL, "flexcan.0");
	clk_register_clkdev(clk[can2_gate], NULL, "flexcan.1");
	clk_register_clkdev(clk[cspi1_gate], "per", "imx35-cspi.0");
	clk_register_clkdev(clk[cspi1_gate], "ipg", "imx35-cspi.0");
	clk_register_clkdev(clk[cspi2_gate], "per", "imx35-cspi.1");
	clk_register_clkdev(clk[cspi2_gate], "ipg", "imx35-cspi.1");
	clk_register_clkdev(clk[epit1_gate], NULL, "imx-epit.0");
	clk_register_clkdev(clk[epit2_gate], NULL, "imx-epit.1");
	clk_register_clkdev(clk[esdhc1_gate], "per", "sdhci-esdhc-imx35.0");
	clk_register_clkdev(clk[ipg], "ipg", "sdhci-esdhc-imx35.0");
	clk_register_clkdev(clk[ahb], "ahb", "sdhci-esdhc-imx35.0");
	clk_register_clkdev(clk[esdhc2_gate], "per", "sdhci-esdhc-imx35.1");
	clk_register_clkdev(clk[ipg], "ipg", "sdhci-esdhc-imx35.1");
	clk_register_clkdev(clk[ahb], "ahb", "sdhci-esdhc-imx35.1");
	clk_register_clkdev(clk[esdhc3_gate], "per", "sdhci-esdhc-imx35.2");
	clk_register_clkdev(clk[ipg], "ipg", "sdhci-esdhc-imx35.2");
	clk_register_clkdev(clk[ahb], "ahb", "sdhci-esdhc-imx35.2");
	/* i.mx35 has the i.mx27 type fec */
	clk_register_clkdev(clk[fec_gate], NULL, "imx27-fec.0");
	clk_register_clkdev(clk[gpt_gate], "per", "imx-gpt.0");
	clk_register_clkdev(clk[ipg], "ipg", "imx-gpt.0");
	clk_register_clkdev(clk[i2c1_gate], NULL, "imx21-i2c.0");
	clk_register_clkdev(clk[i2c2_gate], NULL, "imx21-i2c.1");
	clk_register_clkdev(clk[i2c3_gate], NULL, "imx21-i2c.2");
	clk_register_clkdev(clk[ipu_gate], NULL, "ipu-core");
	clk_register_clkdev(clk[ipu_gate], NULL, "mx3_sdc_fb");
	clk_register_clkdev(clk[kpp_gate], NULL, "imx-keypad");
	clk_register_clkdev(clk[owire_gate], NULL, "mxc_w1");
	clk_register_clkdev(clk[sdma_gate], NULL, "imx35-sdma");
	clk_register_clkdev(clk[ssi1_gate], NULL, "imx-ssi.0");
	clk_register_clkdev(clk[ssi2_gate], NULL, "imx-ssi.1");
	/* i.mx35 has the i.mx21 type uart */
	clk_register_clkdev(clk[uart1_gate], "per", "imx21-uart.0");
	clk_register_clkdev(clk[ipg], "ipg", "imx21-uart.0");
	clk_register_clkdev(clk[uart2_gate], "per", "imx21-uart.1");
	clk_register_clkdev(clk[ipg], "ipg", "imx21-uart.1");
	clk_register_clkdev(clk[uart3_gate], "per", "imx21-uart.2");
	clk_register_clkdev(clk[ipg], "ipg", "imx21-uart.2");
	clk_register_clkdev(clk[usb_div], "per", "mxc-ehci.0");
	clk_register_clkdev(clk[ipg], "ipg", "mxc-ehci.0");
	clk_register_clkdev(clk[usbotg_gate], "ahb", "mxc-ehci.0");
	clk_register_clkdev(clk[usb_div], "per", "mxc-ehci.1");
	clk_register_clkdev(clk[ipg], "ipg", "mxc-ehci.1");
	clk_register_clkdev(clk[usbotg_gate], "ahb", "mxc-ehci.1");
	clk_register_clkdev(clk[usb_div], "per", "mxc-ehci.2");
	clk_register_clkdev(clk[ipg], "ipg", "mxc-ehci.2");
	clk_register_clkdev(clk[usbotg_gate], "ahb", "mxc-ehci.2");
	clk_register_clkdev(clk[usb_div], "per", "imx-udc-mx27");
	clk_register_clkdev(clk[ipg], "ipg", "imx-udc-mx27");
	clk_register_clkdev(clk[usbotg_gate], "ahb", "imx-udc-mx27");
	clk_register_clkdev(clk[wdog_gate], NULL, "imx2-wdt.0");
	clk_register_clkdev(clk[nfc_div], NULL, "imx25-nand.0");
	clk_register_clkdev(clk[csi_gate], NULL, "mx3-camera.0");

	clk_prepare_enable(clk[spba_gate]);
	clk_prepare_enable(clk[gpio1_gate]);
	clk_prepare_enable(clk[gpio2_gate]);
	clk_prepare_enable(clk[gpio3_gate]);
	clk_prepare_enable(clk[iim_gate]);
	clk_prepare_enable(clk[emi_gate]);
	clk_prepare_enable(clk[max_gate]);

	/*
	 * SCC is needed to boot via mmc after a watchdog reset. The clock code
	 * before conversion to common clk also enabled UART1 (which isn't
	 * handled here and not needed for mmc) and IIM (which is enabled
	 * unconditionally above).
	 */
	clk_prepare_enable(clk[scc_gate]);

	imx_print_silicon_rev("i.MX35", mx35_revision());

#ifdef CONFIG_MXC_USE_EPIT
	epit_timer_init(MX35_IO_ADDRESS(MX35_EPIT1_BASE_ADDR), MX35_INT_EPIT1);
#else
	mxc_timer_init(MX35_IO_ADDRESS(MX35_GPT1_BASE_ADDR), MX35_INT_GPT);
#endif

	return 0;
}