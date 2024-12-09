}
EXPORT_SYMBOL(ath9k_hw_intrpend);

void ath9k_hw_kill_interrupts(struct ath_hw *ah)
{
	struct ath_common *common = ath9k_hw_common(ah);

	ath_dbg(common, INTERRUPT, "disable IER\n");
	REG_WRITE(ah, AR_IER, AR_IER_DISABLE);
	(void) REG_READ(ah, AR_IER);
	if (!AR_SREV_9100(ah)) {
		(void) REG_READ(ah, AR_INTR_SYNC_ENABLE);
	}
}
EXPORT_SYMBOL(ath9k_hw_kill_interrupts);

void ath9k_hw_disable_interrupts(struct ath_hw *ah)
{
	if (!(ah->imask & ATH9K_INT_GLOBAL))
		atomic_set(&ah->intr_ref_cnt, -1);
	else
		atomic_dec(&ah->intr_ref_cnt);

	ath9k_hw_kill_interrupts(ah);
}
EXPORT_SYMBOL(ath9k_hw_disable_interrupts);

void ath9k_hw_enable_interrupts(struct ath_hw *ah)
{