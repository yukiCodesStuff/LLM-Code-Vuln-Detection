{
	u32 host_isr;

	if (AR_SREV_9100(ah))
		return true;

	host_isr = REG_READ(ah, AR_INTR_ASYNC_CAUSE);

	if (((host_isr & AR_INTR_MAC_IRQ) ||
	     (host_isr & AR_INTR_ASYNC_MASK_MCI)) &&
	    (host_isr != AR_INTR_SPURIOUS))
		return true;

	host_isr = REG_READ(ah, AR_INTR_SYNC_CAUSE);
	if ((host_isr & AR_INTR_SYNC_DEFAULT)
	    && (host_isr != AR_INTR_SPURIOUS))
		return true;

	return false;
}
EXPORT_SYMBOL(ath9k_hw_intrpend);

void ath9k_hw_disable_interrupts(struct ath_hw *ah)
{
	struct ath_common *common = ath9k_hw_common(ah);

	if (!(ah->imask & ATH9K_INT_GLOBAL))
		atomic_set(&ah->intr_ref_cnt, -1);
	else
		atomic_dec(&ah->intr_ref_cnt);

	ath_dbg(common, INTERRUPT, "disable IER\n");
	REG_WRITE(ah, AR_IER, AR_IER_DISABLE);
	(void) REG_READ(ah, AR_IER);
	if (!AR_SREV_9100(ah)) {
		REG_WRITE(ah, AR_INTR_ASYNC_ENABLE, 0);
		(void) REG_READ(ah, AR_INTR_ASYNC_ENABLE);

		REG_WRITE(ah, AR_INTR_SYNC_ENABLE, 0);
		(void) REG_READ(ah, AR_INTR_SYNC_ENABLE);
	}
}
	if (!AR_SREV_9100(ah)) {
		REG_WRITE(ah, AR_INTR_ASYNC_ENABLE, 0);
		(void) REG_READ(ah, AR_INTR_ASYNC_ENABLE);

		REG_WRITE(ah, AR_INTR_SYNC_ENABLE, 0);
		(void) REG_READ(ah, AR_INTR_SYNC_ENABLE);
	}
}
EXPORT_SYMBOL(ath9k_hw_disable_interrupts);

void ath9k_hw_enable_interrupts(struct ath_hw *ah)
{
	struct ath_common *common = ath9k_hw_common(ah);
	u32 sync_default = AR_INTR_SYNC_DEFAULT;
	u32 async_mask;

	if (!(ah->imask & ATH9K_INT_GLOBAL))
		return;

	if (!atomic_inc_and_test(&ah->intr_ref_cnt)) {
		ath_dbg(common, INTERRUPT, "Do not enable IER ref count %d\n",
			atomic_read(&ah->intr_ref_cnt));
		return;
	}

	if (AR_SREV_9340(ah) || AR_SREV_9550(ah))
		sync_default &= ~AR_INTR_SYNC_HOST1_FATAL;

	async_mask = AR_INTR_MAC_IRQ;

	if (ah->imask & ATH9K_INT_MCI)
		async_mask |= AR_INTR_ASYNC_MASK_MCI;

	ath_dbg(common, INTERRUPT, "enable IER\n");
	REG_WRITE(ah, AR_IER, AR_IER_ENABLE);
	if (!AR_SREV_9100(ah)) {
		REG_WRITE(ah, AR_INTR_ASYNC_ENABLE, async_mask);
		REG_WRITE(ah, AR_INTR_ASYNC_MASK, async_mask);

		REG_WRITE(ah, AR_INTR_SYNC_ENABLE, sync_default);
		REG_WRITE(ah, AR_INTR_SYNC_MASK, sync_default);
	}
	ath_dbg(common, INTERRUPT, "AR_IMR 0x%x IER 0x%x\n",
		REG_READ(ah, AR_IMR), REG_READ(ah, AR_IER));
}
EXPORT_SYMBOL(ath9k_hw_enable_interrupts);

void ath9k_hw_set_interrupts(struct ath_hw *ah)
{
	enum ath9k_int ints = ah->imask;
	u32 mask, mask2;
	struct ath9k_hw_capabilities *pCap = &ah->caps;
	struct ath_common *common = ath9k_hw_common(ah);

	if (!(ints & ATH9K_INT_GLOBAL))
		ath9k_hw_disable_interrupts(ah);

	ath_dbg(common, INTERRUPT, "New interrupt mask 0x%x\n", ints);

	mask = ints & ATH9K_INT_COMMON;
	mask2 = 0;

	if (ints & ATH9K_INT_TX) {
		if (ah->config.tx_intr_mitigation)
			mask |= AR_IMR_TXMINTR | AR_IMR_TXINTM;
		else {
			if (ah->txok_interrupt_mask)
				mask |= AR_IMR_TXOK;
			if (ah->txdesc_interrupt_mask)
				mask |= AR_IMR_TXDESC;
		}