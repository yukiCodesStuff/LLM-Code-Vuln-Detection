		/* Start bit low is too short, go back to idle */
		if (delta < CEC_TIM_START_BIT_LOW_MIN - CEC_TIM_IDLE_SAMPLE) {
			if (!pin->rx_start_bit_low_too_short_cnt++) {
				pin->rx_start_bit_low_too_short_ts = ktime_to_ns(pin->ts);
				pin->rx_start_bit_low_too_short_delta = delta;
			}
			cec_pin_to_idle(pin);
			break;
		/* Start bit is too short, go back to idle */
		if (delta < CEC_TIM_START_BIT_TOTAL_MIN - CEC_TIM_IDLE_SAMPLE) {
			if (!pin->rx_start_bit_too_short_cnt++) {
				pin->rx_start_bit_too_short_ts = ktime_to_ns(pin->ts);
				pin->rx_start_bit_too_short_delta = delta;
			}
			cec_pin_to_idle(pin);
			break;
		 */
		if (delta < CEC_TIM_DATA_BIT_TOTAL_MIN) {
			if (!pin->rx_data_bit_too_short_cnt++) {
				pin->rx_data_bit_too_short_ts = ktime_to_ns(pin->ts);
				pin->rx_data_bit_too_short_delta = delta;
			}
			cec_pin_low(pin);
			pin->state = CEC_ST_RX_LOW_DRIVE;