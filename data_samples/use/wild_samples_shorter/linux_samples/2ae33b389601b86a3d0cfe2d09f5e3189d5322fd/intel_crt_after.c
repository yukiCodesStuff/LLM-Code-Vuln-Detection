	u32 temp;

	temp = I915_READ(crt->adpa_reg);
	temp |= ADPA_HSYNC_CNTL_DISABLE | ADPA_VSYNC_CNTL_DISABLE;
	temp &= ~ADPA_DAC_ENABLE;
	I915_WRITE(crt->adpa_reg, temp);
}
