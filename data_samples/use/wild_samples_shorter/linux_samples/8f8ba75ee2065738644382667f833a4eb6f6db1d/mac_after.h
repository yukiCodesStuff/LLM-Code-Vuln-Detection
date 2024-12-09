void ath9k_hw_set_interrupts(struct ath_hw *ah);
void ath9k_hw_enable_interrupts(struct ath_hw *ah);
void ath9k_hw_disable_interrupts(struct ath_hw *ah);
void ath9k_hw_kill_interrupts(struct ath_hw *ah);

void ar9002_hw_attach_mac_ops(struct ath_hw *ah);

#endif /* MAC_H */