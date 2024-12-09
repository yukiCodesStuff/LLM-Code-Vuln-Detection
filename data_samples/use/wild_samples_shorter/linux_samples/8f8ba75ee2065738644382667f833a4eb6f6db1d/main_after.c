	if (!ath9k_hw_intrpend(ah))
		return IRQ_NONE;

	if (test_bit(SC_OP_HW_RESET, &sc->sc_flags)) {
		ath9k_hw_kill_interrupts(ah);
		return IRQ_HANDLED;
	}

	/*
	 * Figure out the reason(s) for the interrupt.  Note
	 * that the hal returns a pseudo-ISR that may include