				mask2 |= ATH9K_INT_CST;
			if (isr2 & AR_ISR_S2_TSFOOR)
				mask2 |= ATH9K_INT_TSFOOR;

			if (!(pCap->hw_caps & ATH9K_HW_CAP_RAC_SUPPORTED)) {
				REG_WRITE(ah, AR_ISR_S2, isr2);
				isr &= ~AR_ISR_BCNMISC;
			}
		}

		if (pCap->hw_caps & ATH9K_HW_CAP_RAC_SUPPORTED)
			isr = REG_READ(ah, AR_ISR_RAC);

		if (isr == 0xffffffff) {
			*masked = 0;
			return false;
		}

			*masked |= ATH9K_INT_TX;

			if (pCap->hw_caps & ATH9K_HW_CAP_RAC_SUPPORTED) {
				s0_s = REG_READ(ah, AR_ISR_S0_S);
				s1_s = REG_READ(ah, AR_ISR_S1_S);
			} else {
				s0_s = REG_READ(ah, AR_ISR_S0);
				REG_WRITE(ah, AR_ISR_S0, s0_s);
				s1_s = REG_READ(ah, AR_ISR_S1);
				REG_WRITE(ah, AR_ISR_S1, s1_s);

				isr &= ~(AR_ISR_TXOK |
					 AR_ISR_TXDESC |
					 AR_ISR_TXERR |
					 AR_ISR_TXEOL);
			}

			ah->intr_txqs |= MS(s0_s, AR_ISR_S0_QCU_TXOK);
			ah->intr_txqs |= MS(s0_s, AR_ISR_S0_QCU_TXDESC);
			ah->intr_txqs |= MS(s1_s, AR_ISR_S1_QCU_TXERR);
			ah->intr_txqs |= MS(s1_s, AR_ISR_S1_QCU_TXEOL);
		}

		*masked |= mask2;
	}

	if (!AR_SREV_9100(ah) && (isr & AR_ISR_GENTMR)) {
		u32 s5_s;

		if (pCap->hw_caps & ATH9K_HW_CAP_RAC_SUPPORTED) {
			s5_s = REG_READ(ah, AR_ISR_S5_S);
		} else {
			s5_s = REG_READ(ah, AR_ISR_S5);
		}

		ah->intr_gen_timer_trigger =
				MS(s5_s, AR_ISR_S5_GENTIMER_TRIG);

		ah->intr_gen_timer_thresh =
		if ((s5_s & AR_ISR_S5_TIM_TIMER) &&
		    !(pCap->hw_caps & ATH9K_HW_CAP_AUTOSLEEP))
			*masked |= ATH9K_INT_TIM_TIMER;

		if (!(pCap->hw_caps & ATH9K_HW_CAP_RAC_SUPPORTED)) {
			REG_WRITE(ah, AR_ISR_S5, s5_s);
			isr &= ~AR_ISR_GENTMR;
		}
	}

	if (!(pCap->hw_caps & ATH9K_HW_CAP_RAC_SUPPORTED)) {
		REG_WRITE(ah, AR_ISR, isr);
		REG_READ(ah, AR_ISR);
	}

	if (AR_SREV_9100(ah))
		return true;

	if (sync_cause) {
		ath9k_debug_sync_cause(common, sync_cause);
		fatal_int =
			(sync_cause &