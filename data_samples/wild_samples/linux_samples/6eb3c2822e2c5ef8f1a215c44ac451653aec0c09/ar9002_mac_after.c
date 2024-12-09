		if (isr & AR_ISR_BCNMISC) {
			u32 isr2;
			isr2 = REG_READ(ah, AR_ISR_S2);
			if (isr2 & AR_ISR_S2_TIM)
				mask2 |= ATH9K_INT_TIM;
			if (isr2 & AR_ISR_S2_DTIM)
				mask2 |= ATH9K_INT_DTIM;
			if (isr2 & AR_ISR_S2_DTIMSYNC)
				mask2 |= ATH9K_INT_DTIMSYNC;
			if (isr2 & (AR_ISR_S2_CABEND))
				mask2 |= ATH9K_INT_CABEND;
			if (isr2 & AR_ISR_S2_GTT)
				mask2 |= ATH9K_INT_GTT;
			if (isr2 & AR_ISR_S2_CST)
				mask2 |= ATH9K_INT_CST;
			if (isr2 & AR_ISR_S2_TSFOOR)
				mask2 |= ATH9K_INT_TSFOOR;

			if (!(pCap->hw_caps & ATH9K_HW_CAP_RAC_SUPPORTED)) {
				REG_WRITE(ah, AR_ISR_S2, isr2);
				isr &= ~AR_ISR_BCNMISC;
			}
		     AR_ISR_TXEOL)) {
			u32 s0_s, s1_s;

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
		if (isr & AR_ISR_RXORN) {
			ath_dbg(common, INTERRUPT,
				"receive FIFO overrun interrupt\n");
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
			MS(s5_s, AR_ISR_S5_GENTIMER_THRESH);

		if (ah->intr_gen_timer_trigger)
			*masked |= ATH9K_INT_GENTIMER;

		if ((s5_s & AR_ISR_S5_TIM_TIMER) &&
		    !(pCap->hw_caps & ATH9K_HW_CAP_AUTOSLEEP))
			*masked |= ATH9K_INT_TIM_TIMER;

		if (!(pCap->hw_caps & ATH9K_HW_CAP_RAC_SUPPORTED)) {
			REG_WRITE(ah, AR_ISR_S5, s5_s);
			isr &= ~AR_ISR_GENTMR;
		}
	}
		} else {
			s5_s = REG_READ(ah, AR_ISR_S5);
		}

		ah->intr_gen_timer_trigger =
				MS(s5_s, AR_ISR_S5_GENTIMER_TRIG);

		ah->intr_gen_timer_thresh =
			MS(s5_s, AR_ISR_S5_GENTIMER_THRESH);

		if (ah->intr_gen_timer_trigger)
			*masked |= ATH9K_INT_GENTIMER;

		if ((s5_s & AR_ISR_S5_TIM_TIMER) &&
		    !(pCap->hw_caps & ATH9K_HW_CAP_AUTOSLEEP))
			*masked |= ATH9K_INT_TIM_TIMER;

		if (!(pCap->hw_caps & ATH9K_HW_CAP_RAC_SUPPORTED)) {
			REG_WRITE(ah, AR_ISR_S5, s5_s);
			isr &= ~AR_ISR_GENTMR;
		}