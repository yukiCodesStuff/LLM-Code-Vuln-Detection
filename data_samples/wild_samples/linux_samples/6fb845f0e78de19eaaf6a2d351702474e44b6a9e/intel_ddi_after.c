		switch (clock) {
		case 162000:
			return DDI_CLK_SEL_TBT_162;
		case 270000:
			return DDI_CLK_SEL_TBT_270;
		case 540000:
			return DDI_CLK_SEL_TBT_540;
		case 810000:
			return DDI_CLK_SEL_TBT_810;
		default:
			MISSING_CASE(clock);
			return DDI_CLK_SEL_NONE;
		}
	case DPLL_ID_ICL_MGPLL1:
	case DPLL_ID_ICL_MGPLL2:
	case DPLL_ID_ICL_MGPLL3:
	case DPLL_ID_ICL_MGPLL4:
		return DDI_CLK_SEL_MG;
	}
}

/* Starting with Haswell, different DDI ports can work in FDI mode for
 * connection to the PCH-located connectors. For this, it is necessary to train
 * both the DDI port and PCH receiver for the desired DDI buffer settings.
 *
 * The recommended port to work in FDI mode is DDI E, which we use here. Also,
 * please note that when FDI mode is active on DDI E, it shares 2 lines with
 * DDI A (which is used for eDP)
 */

void hsw_fdi_link_train(struct intel_crtc *crtc,
			const struct intel_crtc_state *crtc_state)
{
	struct drm_device *dev = crtc->base.dev;
	struct drm_i915_private *dev_priv = to_i915(dev);
	struct intel_encoder *encoder;
	u32 temp, i, rx_ctl_val, ddi_pll_sel;

	for_each_encoder_on_crtc(dev, &crtc->base, encoder) {
		WARN_ON(encoder->type != INTEL_OUTPUT_ANALOG);
		intel_prepare_dp_ddi_buffers(encoder, crtc_state);
	}

	/* Set the FDI_RX_MISC pwrdn lanes and the 2 workarounds listed at the
	 * mode set "sequence for CRT port" document:
	 * - TP1 to TP2 time with the default value
	 * - FDI delay to 90h
	 *
	 * WaFDIAutoLinkSetTimingOverrride:hsw
	 */
	I915_WRITE(FDI_RX_MISC(PIPE_A), FDI_RX_PWRDN_LANE1_VAL(2) |
				  FDI_RX_PWRDN_LANE0_VAL(2) |
				  FDI_RX_TP1_TO_TP2_48 | FDI_RX_FDI_DELAY_90);

	/* Enable the PCH Receiver FDI PLL */
	rx_ctl_val = dev_priv->fdi_rx_config | FDI_RX_ENHANCE_FRAME_ENABLE |
		     FDI_RX_PLL_ENABLE |
		     FDI_DP_PORT_WIDTH(crtc_state->fdi_lanes);
	I915_WRITE(FDI_RX_CTL(PIPE_A), rx_ctl_val);
	POSTING_READ(FDI_RX_CTL(PIPE_A));
	udelay(220);

	/* Switch from Rawclk to PCDclk */
	rx_ctl_val |= FDI_PCDCLK;
	I915_WRITE(FDI_RX_CTL(PIPE_A), rx_ctl_val);

	/* Configure Port Clock Select */
	ddi_pll_sel = hsw_pll_to_ddi_pll_sel(crtc_state->shared_dpll);
	I915_WRITE(PORT_CLK_SEL(PORT_E), ddi_pll_sel);
	WARN_ON(ddi_pll_sel != PORT_CLK_SEL_SPLL);

	/* Start the training iterating through available voltages and emphasis,
	 * testing each value twice. */
	for (i = 0; i < ARRAY_SIZE(hsw_ddi_translations_fdi) * 2; i++) {
		/* Configure DP_TP_CTL with auto-training */
		I915_WRITE(DP_TP_CTL(PORT_E),
					DP_TP_CTL_FDI_AUTOTRAIN |
					DP_TP_CTL_ENHANCED_FRAME_ENABLE |
					DP_TP_CTL_LINK_TRAIN_PAT1 |
					DP_TP_CTL_ENABLE);

		/* Configure and enable DDI_BUF_CTL for DDI E with next voltage.
		 * DDI E does not support port reversal, the functionality is
		 * achieved on the PCH side in FDI_RX_CTL, so no need to set the
		 * port reversal bit */
		I915_WRITE(DDI_BUF_CTL(PORT_E),
			   DDI_BUF_CTL_ENABLE |
			   ((crtc_state->fdi_lanes - 1) << 1) |
			   DDI_BUF_TRANS_SELECT(i / 2));
		POSTING_READ(DDI_BUF_CTL(PORT_E));

		udelay(600);

		/* Program PCH FDI Receiver TU */
		I915_WRITE(FDI_RX_TUSIZE1(PIPE_A), TU_SIZE(64));

		/* Enable PCH FDI Receiver with auto-training */
		rx_ctl_val |= FDI_RX_ENABLE | FDI_LINK_TRAIN_AUTO;
		I915_WRITE(FDI_RX_CTL(PIPE_A), rx_ctl_val);
		POSTING_READ(FDI_RX_CTL(PIPE_A));

		/* Wait for FDI receiver lane calibration */
		udelay(30);

		/* Unset FDI_RX_MISC pwrdn lanes */
		temp = I915_READ(FDI_RX_MISC(PIPE_A));
		temp &= ~(FDI_RX_PWRDN_LANE1_MASK | FDI_RX_PWRDN_LANE0_MASK);
		I915_WRITE(FDI_RX_MISC(PIPE_A), temp);
		POSTING_READ(FDI_RX_MISC(PIPE_A));

		/* Wait for FDI auto training time */
		udelay(5);

		temp = I915_READ(DP_TP_STATUS(PORT_E));
		if (temp & DP_TP_STATUS_AUTOTRAIN_DONE) {
			DRM_DEBUG_KMS("FDI link training done on step %d\n", i);
			break;
		}

		/*
		 * Leave things enabled even if we failed to train FDI.
		 * Results in less fireworks from the state checker.
		 */
		if (i == ARRAY_SIZE(hsw_ddi_translations_fdi) * 2 - 1) {
			DRM_ERROR("FDI link training failed!\n");
			break;
		}

		rx_ctl_val &= ~FDI_RX_ENABLE;
		I915_WRITE(FDI_RX_CTL(PIPE_A), rx_ctl_val);
		POSTING_READ(FDI_RX_CTL(PIPE_A));

		temp = I915_READ(DDI_BUF_CTL(PORT_E));
		temp &= ~DDI_BUF_CTL_ENABLE;
		I915_WRITE(DDI_BUF_CTL(PORT_E), temp);
		POSTING_READ(DDI_BUF_CTL(PORT_E));

		/* Disable DP_TP_CTL and FDI_RX_CTL and retry */
		temp = I915_READ(DP_TP_CTL(PORT_E));
		temp &= ~(DP_TP_CTL_ENABLE | DP_TP_CTL_LINK_TRAIN_MASK);
		temp |= DP_TP_CTL_LINK_TRAIN_PAT1;
		I915_WRITE(DP_TP_CTL(PORT_E), temp);
		POSTING_READ(DP_TP_CTL(PORT_E));

		intel_wait_ddi_buf_idle(dev_priv, PORT_E);

		/* Reset FDI_RX_MISC pwrdn lanes */
		temp = I915_READ(FDI_RX_MISC(PIPE_A));
		temp &= ~(FDI_RX_PWRDN_LANE1_MASK | FDI_RX_PWRDN_LANE0_MASK);
		temp |= FDI_RX_PWRDN_LANE1_VAL(2) | FDI_RX_PWRDN_LANE0_VAL(2);
		I915_WRITE(FDI_RX_MISC(PIPE_A), temp);
		POSTING_READ(FDI_RX_MISC(PIPE_A));
	}

	/* Enable normal pixel sending for FDI */
	I915_WRITE(DP_TP_CTL(PORT_E),
		   DP_TP_CTL_FDI_AUTOTRAIN |
		   DP_TP_CTL_LINK_TRAIN_NORMAL |
		   DP_TP_CTL_ENHANCED_FRAME_ENABLE |
		   DP_TP_CTL_ENABLE);
}

static void intel_ddi_init_dp_buf_reg(struct intel_encoder *encoder)
{
	struct intel_dp *intel_dp = enc_to_intel_dp(&encoder->base);
	struct intel_digital_port *intel_dig_port =
		enc_to_dig_port(&encoder->base);

	intel_dp->DP = intel_dig_port->saved_port_bits |
		DDI_BUF_CTL_ENABLE | DDI_BUF_TRANS_SELECT(0);
	intel_dp->DP |= DDI_PORT_WIDTH(intel_dp->lane_count);
}

static struct intel_encoder *
intel_ddi_get_crtc_encoder(struct intel_crtc *crtc)
{
	struct drm_device *dev = crtc->base.dev;
	struct intel_encoder *encoder, *ret = NULL;
	int num_encoders = 0;

	for_each_encoder_on_crtc(dev, &crtc->base, encoder) {
		ret = encoder;
		num_encoders++;
	}

	if (num_encoders != 1)
		WARN(1, "%d encoders on crtc for pipe %c\n", num_encoders,
		     pipe_name(crtc->pipe));

	BUG_ON(ret == NULL);
	return ret;
}

#define LC_FREQ 2700

static int hsw_ddi_calc_wrpll_link(struct drm_i915_private *dev_priv,
				   i915_reg_t reg)
{
	int refclk = LC_FREQ;
	int n, p, r;
	u32 wrpll;

	wrpll = I915_READ(reg);
	switch (wrpll & WRPLL_PLL_REF_MASK) {
	case WRPLL_PLL_SSC:
	case WRPLL_PLL_NON_SSC:
		/*
		 * We could calculate spread here, but our checking
		 * code only cares about 5% accuracy, and spread is a max of
		 * 0.5% downspread.
		 */
		refclk = 135;
		break;
	case WRPLL_PLL_LCPLL:
		refclk = LC_FREQ;
		break;
	default:
		WARN(1, "bad wrpll refclk\n");
		return 0;
	}

	r = wrpll & WRPLL_DIVIDER_REF_MASK;
	p = (wrpll & WRPLL_DIVIDER_POST_MASK) >> WRPLL_DIVIDER_POST_SHIFT;
	n = (wrpll & WRPLL_DIVIDER_FB_MASK) >> WRPLL_DIVIDER_FB_SHIFT;

	/* Convert to KHz, p & r have a fixed point portion */
	return (refclk * n * 100) / (p * r);
}

static int skl_calc_wrpll_link(struct drm_i915_private *dev_priv,
			       enum intel_dpll_id pll_id)
{
	i915_reg_t cfgcr1_reg, cfgcr2_reg;
	uint32_t cfgcr1_val, cfgcr2_val;
	uint32_t p0, p1, p2, dco_freq;

	cfgcr1_reg = DPLL_CFGCR1(pll_id);
	cfgcr2_reg = DPLL_CFGCR2(pll_id);

	cfgcr1_val = I915_READ(cfgcr1_reg);
	cfgcr2_val = I915_READ(cfgcr2_reg);

	p0 = cfgcr2_val & DPLL_CFGCR2_PDIV_MASK;
	p2 = cfgcr2_val & DPLL_CFGCR2_KDIV_MASK;

	if (cfgcr2_val &  DPLL_CFGCR2_QDIV_MODE(1))
		p1 = (cfgcr2_val & DPLL_CFGCR2_QDIV_RATIO_MASK) >> 8;
	else
		p1 = 1;


	switch (p0) {
	case DPLL_CFGCR2_PDIV_1:
		p0 = 1;
		break;
	case DPLL_CFGCR2_PDIV_2:
		p0 = 2;
		break;
	case DPLL_CFGCR2_PDIV_3:
		p0 = 3;
		break;
	case DPLL_CFGCR2_PDIV_7:
		p0 = 7;
		break;
	}

	switch (p2) {
	case DPLL_CFGCR2_KDIV_5:
		p2 = 5;
		break;
	case DPLL_CFGCR2_KDIV_2:
		p2 = 2;
		break;
	case DPLL_CFGCR2_KDIV_3:
		p2 = 3;
		break;
	case DPLL_CFGCR2_KDIV_1:
		p2 = 1;
		break;
	}

	dco_freq = (cfgcr1_val & DPLL_CFGCR1_DCO_INTEGER_MASK) * 24 * 1000;

	dco_freq += (((cfgcr1_val & DPLL_CFGCR1_DCO_FRACTION_MASK) >> 9) * 24 *
		1000) / 0x8000;

	return dco_freq / (p0 * p1 * p2 * 5);
}

int cnl_calc_wrpll_link(struct drm_i915_private *dev_priv,
			enum intel_dpll_id pll_id)
{
	uint32_t cfgcr0, cfgcr1;
	uint32_t p0, p1, p2, dco_freq, ref_clock;

	if (INTEL_GEN(dev_priv) >= 11) {
		cfgcr0 = I915_READ(ICL_DPLL_CFGCR0(pll_id));
		cfgcr1 = I915_READ(ICL_DPLL_CFGCR1(pll_id));
	} else {
		cfgcr0 = I915_READ(CNL_DPLL_CFGCR0(pll_id));
		cfgcr1 = I915_READ(CNL_DPLL_CFGCR1(pll_id));
	}

	p0 = cfgcr1 & DPLL_CFGCR1_PDIV_MASK;
	p2 = cfgcr1 & DPLL_CFGCR1_KDIV_MASK;

	if (cfgcr1 & DPLL_CFGCR1_QDIV_MODE(1))
		p1 = (cfgcr1 & DPLL_CFGCR1_QDIV_RATIO_MASK) >>
			DPLL_CFGCR1_QDIV_RATIO_SHIFT;
	else
		p1 = 1;


	switch (p0) {
	case DPLL_CFGCR1_PDIV_2:
		p0 = 2;
		break;
	case DPLL_CFGCR1_PDIV_3:
		p0 = 3;
		break;
	case DPLL_CFGCR1_PDIV_5:
		p0 = 5;
		break;
	case DPLL_CFGCR1_PDIV_7:
		p0 = 7;
		break;
	}

	switch (p2) {
	case DPLL_CFGCR1_KDIV_1:
		p2 = 1;
		break;
	case DPLL_CFGCR1_KDIV_2:
		p2 = 2;
		break;
	case DPLL_CFGCR1_KDIV_4:
		p2 = 4;
		break;
	}

	ref_clock = cnl_hdmi_pll_ref_clock(dev_priv);

	dco_freq = (cfgcr0 & DPLL_CFGCR0_DCO_INTEGER_MASK) * ref_clock;

	dco_freq += (((cfgcr0 & DPLL_CFGCR0_DCO_FRACTION_MASK) >>
		      DPLL_CFGCR0_DCO_FRACTION_SHIFT) * ref_clock) / 0x8000;

	if (WARN_ON(p0 == 0 || p1 == 0 || p2 == 0))
		return 0;

	return dco_freq / (p0 * p1 * p2 * 5);
}

static int icl_calc_tbt_pll_link(struct drm_i915_private *dev_priv,
				 enum port port)
{
	u32 val = I915_READ(DDI_CLK_SEL(port)) & DDI_CLK_SEL_MASK;

	switch (val) {
	case DDI_CLK_SEL_NONE:
		return 0;
	case DDI_CLK_SEL_TBT_162:
		return 162000;
	case DDI_CLK_SEL_TBT_270:
		return 270000;
	case DDI_CLK_SEL_TBT_540:
		return 540000;
	case DDI_CLK_SEL_TBT_810:
		return 810000;
	default:
		MISSING_CASE(val);
		return 0;
	}
}

static int icl_calc_mg_pll_link(struct drm_i915_private *dev_priv,
				enum port port)
{
	u32 mg_pll_div0, mg_clktop_hsclkctl;
	u32 m1, m2_int, m2_frac, div1, div2, refclk;
	u64 tmp;

	refclk = dev_priv->cdclk.hw.ref;

	mg_pll_div0 = I915_READ(MG_PLL_DIV0(port));
	mg_clktop_hsclkctl = I915_READ(MG_CLKTOP2_HSCLKCTL(port));

	m1 = I915_READ(MG_PLL_DIV1(port)) & MG_PLL_DIV1_FBPREDIV_MASK;
	m2_int = mg_pll_div0 & MG_PLL_DIV0_FBDIV_INT_MASK;
	m2_frac = (mg_pll_div0 & MG_PLL_DIV0_FRACNEN_H) ?
		  (mg_pll_div0 & MG_PLL_DIV0_FBDIV_FRAC_MASK) >>
		  MG_PLL_DIV0_FBDIV_FRAC_SHIFT : 0;

	switch (mg_clktop_hsclkctl & MG_CLKTOP2_HSCLKCTL_HSDIV_RATIO_MASK) {
	case MG_CLKTOP2_HSCLKCTL_HSDIV_RATIO_2:
		div1 = 2;
		break;
	case MG_CLKTOP2_HSCLKCTL_HSDIV_RATIO_3:
		div1 = 3;
		break;
	case MG_CLKTOP2_HSCLKCTL_HSDIV_RATIO_5:
		div1 = 5;
		break;
	case MG_CLKTOP2_HSCLKCTL_HSDIV_RATIO_7:
		div1 = 7;
		break;
	default:
		MISSING_CASE(mg_clktop_hsclkctl);
		return 0;
	}

	div2 = (mg_clktop_hsclkctl & MG_CLKTOP2_HSCLKCTL_DSDIV_RATIO_MASK) >>
		MG_CLKTOP2_HSCLKCTL_DSDIV_RATIO_SHIFT;
	/* div2 value of 0 is same as 1 means no div */
	if (div2 == 0)
		div2 = 1;

	/*
	 * Adjust the original formula to delay the division by 2^22 in order to
	 * minimize possible rounding errors.
	 */
	tmp = (u64)m1 * m2_int * refclk +
	      (((u64)m1 * m2_frac * refclk) >> 22);
	tmp = div_u64(tmp, 5 * div1 * div2);

	return tmp;
}

static void ddi_dotclock_get(struct intel_crtc_state *pipe_config)
{
	int dotclock;

	if (pipe_config->has_pch_encoder)
		dotclock = intel_dotclock_calculate(pipe_config->port_clock,
						    &pipe_config->fdi_m_n);
	else if (intel_crtc_has_dp_encoder(pipe_config))
		dotclock = intel_dotclock_calculate(pipe_config->port_clock,
						    &pipe_config->dp_m_n);
	else if (pipe_config->has_hdmi_sink && pipe_config->pipe_bpp == 36)
		dotclock = pipe_config->port_clock * 2 / 3;
	else
		dotclock = pipe_config->port_clock;

	if (pipe_config->output_format == INTEL_OUTPUT_FORMAT_YCBCR420)
		dotclock *= 2;

	if (pipe_config->pixel_multiplier)
		dotclock /= pipe_config->pixel_multiplier;

	pipe_config->base.adjusted_mode.crtc_clock = dotclock;
}

static void icl_ddi_clock_get(struct intel_encoder *encoder,
			      struct intel_crtc_state *pipe_config)
{
	struct drm_i915_private *dev_priv = to_i915(encoder->base.dev);
	enum port port = encoder->port;
	int link_clock = 0;
	uint32_t pll_id;

	pll_id = intel_get_shared_dpll_id(dev_priv, pipe_config->shared_dpll);
	if (intel_port_is_combophy(dev_priv, port)) {
		if (intel_crtc_has_type(pipe_config, INTEL_OUTPUT_HDMI))
			link_clock = cnl_calc_wrpll_link(dev_priv, pll_id);
		else
			link_clock = icl_calc_dp_combo_pll_link(dev_priv,
								pll_id);
	} else {
		if (pll_id == DPLL_ID_ICL_TBTPLL)
			link_clock = icl_calc_tbt_pll_link(dev_priv, port);
		else
			link_clock = icl_calc_mg_pll_link(dev_priv, port);
	}