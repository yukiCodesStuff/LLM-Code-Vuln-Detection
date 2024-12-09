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
		}

		isr = REG_READ(ah, AR_ISR_RAC);
		if (isr == 0xffffffff) {
			*masked = 0;
			return false;
		}

		*masked = isr & ATH9K_INT_COMMON;

		if (isr & (AR_ISR_RXMINTR | AR_ISR_RXINTM |
			   AR_ISR_RXOK | AR_ISR_RXERR))
			*masked |= ATH9K_INT_RX;

		if (isr &
		    (AR_ISR_TXOK | AR_ISR_TXDESC | AR_ISR_TXERR |
		     AR_ISR_TXEOL)) {
			u32 s0_s, s1_s;

			*masked |= ATH9K_INT_TX;

			s0_s = REG_READ(ah, AR_ISR_S0_S);
			ah->intr_txqs |= MS(s0_s, AR_ISR_S0_QCU_TXOK);
			ah->intr_txqs |= MS(s0_s, AR_ISR_S0_QCU_TXDESC);

			s1_s = REG_READ(ah, AR_ISR_S1_S);
			ah->intr_txqs |= MS(s1_s, AR_ISR_S1_QCU_TXERR);
			ah->intr_txqs |= MS(s1_s, AR_ISR_S1_QCU_TXEOL);
		}

		if (isr & AR_ISR_RXORN) {
			ath_dbg(common, INTERRUPT,
				"receive FIFO overrun interrupt\n");
		}

		*masked |= mask2;
	}

	if (AR_SREV_9100(ah))
		return true;

	if (isr & AR_ISR_GENTMR) {
		u32 s5_s;

		s5_s = REG_READ(ah, AR_ISR_S5_S);
		ah->intr_gen_timer_trigger =
				MS(s5_s, AR_ISR_S5_GENTIMER_TRIG);

		ah->intr_gen_timer_thresh =
			MS(s5_s, AR_ISR_S5_GENTIMER_THRESH);

		if (ah->intr_gen_timer_trigger)
			*masked |= ATH9K_INT_GENTIMER;

		if ((s5_s & AR_ISR_S5_TIM_TIMER) &&
		    !(pCap->hw_caps & ATH9K_HW_CAP_AUTOSLEEP))
			*masked |= ATH9K_INT_TIM_TIMER;
	}

	if (sync_cause) {
		ath9k_debug_sync_cause(common, sync_cause);
		fatal_int =
			(sync_cause &
			 (AR_INTR_SYNC_HOST1_FATAL | AR_INTR_SYNC_HOST1_PERR))
			? true : false;

		if (fatal_int) {
			if (sync_cause & AR_INTR_SYNC_HOST1_FATAL) {
				ath_dbg(common, ANY,
					"received PCI FATAL interrupt\n");
			}
		     AR_ISR_TXEOL)) {
			u32 s0_s, s1_s;

			*masked |= ATH9K_INT_TX;

			s0_s = REG_READ(ah, AR_ISR_S0_S);
			ah->intr_txqs |= MS(s0_s, AR_ISR_S0_QCU_TXOK);
			ah->intr_txqs |= MS(s0_s, AR_ISR_S0_QCU_TXDESC);

			s1_s = REG_READ(ah, AR_ISR_S1_S);
			ah->intr_txqs |= MS(s1_s, AR_ISR_S1_QCU_TXERR);
			ah->intr_txqs |= MS(s1_s, AR_ISR_S1_QCU_TXEOL);
		}

		if (isr & AR_ISR_RXORN) {
			ath_dbg(common, INTERRUPT,
				"receive FIFO overrun interrupt\n");
		}

		*masked |= mask2;
	}

	if (AR_SREV_9100(ah))
		return true;

	if (isr & AR_ISR_GENTMR) {
		u32 s5_s;

		s5_s = REG_READ(ah, AR_ISR_S5_S);
		ah->intr_gen_timer_trigger =
				MS(s5_s, AR_ISR_S5_GENTIMER_TRIG);

		ah->intr_gen_timer_thresh =
			MS(s5_s, AR_ISR_S5_GENTIMER_THRESH);

		if (ah->intr_gen_timer_trigger)
			*masked |= ATH9K_INT_GENTIMER;

		if ((s5_s & AR_ISR_S5_TIM_TIMER) &&
		    !(pCap->hw_caps & ATH9K_HW_CAP_AUTOSLEEP))
			*masked |= ATH9K_INT_TIM_TIMER;
	}

	if (sync_cause) {
		ath9k_debug_sync_cause(common, sync_cause);
		fatal_int =
			(sync_cause &
			 (AR_INTR_SYNC_HOST1_FATAL | AR_INTR_SYNC_HOST1_PERR))
			? true : false;

		if (fatal_int) {
			if (sync_cause & AR_INTR_SYNC_HOST1_FATAL) {
				ath_dbg(common, ANY,
					"received PCI FATAL interrupt\n");
			}
		if (isr & AR_ISR_RXORN) {
			ath_dbg(common, INTERRUPT,
				"receive FIFO overrun interrupt\n");
		}

		*masked |= mask2;
	}

	if (AR_SREV_9100(ah))
		return true;

	if (isr & AR_ISR_GENTMR) {
		u32 s5_s;

		s5_s = REG_READ(ah, AR_ISR_S5_S);
		ah->intr_gen_timer_trigger =
				MS(s5_s, AR_ISR_S5_GENTIMER_TRIG);

		ah->intr_gen_timer_thresh =
			MS(s5_s, AR_ISR_S5_GENTIMER_THRESH);

		if (ah->intr_gen_timer_trigger)
			*masked |= ATH9K_INT_GENTIMER;

		if ((s5_s & AR_ISR_S5_TIM_TIMER) &&
		    !(pCap->hw_caps & ATH9K_HW_CAP_AUTOSLEEP))
			*masked |= ATH9K_INT_TIM_TIMER;
	}

	if (sync_cause) {
		ath9k_debug_sync_cause(common, sync_cause);
		fatal_int =
			(sync_cause &
			 (AR_INTR_SYNC_HOST1_FATAL | AR_INTR_SYNC_HOST1_PERR))
			? true : false;

		if (fatal_int) {
			if (sync_cause & AR_INTR_SYNC_HOST1_FATAL) {
				ath_dbg(common, ANY,
					"received PCI FATAL interrupt\n");
			}
			if (sync_cause & AR_INTR_SYNC_HOST1_PERR) {
				ath_dbg(common, ANY,
					"received PCI PERR interrupt\n");
			}
			*masked |= ATH9K_INT_FATAL;
		}
	if (isr & AR_ISR_GENTMR) {
		u32 s5_s;

		s5_s = REG_READ(ah, AR_ISR_S5_S);
		ah->intr_gen_timer_trigger =
				MS(s5_s, AR_ISR_S5_GENTIMER_TRIG);

		ah->intr_gen_timer_thresh =
			MS(s5_s, AR_ISR_S5_GENTIMER_THRESH);

		if (ah->intr_gen_timer_trigger)
			*masked |= ATH9K_INT_GENTIMER;

		if ((s5_s & AR_ISR_S5_TIM_TIMER) &&
		    !(pCap->hw_caps & ATH9K_HW_CAP_AUTOSLEEP))
			*masked |= ATH9K_INT_TIM_TIMER;
	}

	if (sync_cause) {
		ath9k_debug_sync_cause(common, sync_cause);
		fatal_int =
			(sync_cause &
			 (AR_INTR_SYNC_HOST1_FATAL | AR_INTR_SYNC_HOST1_PERR))
			? true : false;

		if (fatal_int) {
			if (sync_cause & AR_INTR_SYNC_HOST1_FATAL) {
				ath_dbg(common, ANY,
					"received PCI FATAL interrupt\n");
			}
			if (sync_cause & AR_INTR_SYNC_HOST1_PERR) {
				ath_dbg(common, ANY,
					"received PCI PERR interrupt\n");
			}
			*masked |= ATH9K_INT_FATAL;
		}