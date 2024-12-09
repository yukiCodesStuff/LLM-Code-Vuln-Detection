{
	struct mt76_phy *phy = priv;
	bool suspend = test_bit(MT76_STATE_SUSPEND, &phy->state);
	struct ieee80211_hw *hw = phy->hw;
	struct cfg80211_wowlan *wowlan = hw->wiphy->wowlan_config;
	int i;

	mt76_connac_mcu_set_gtk_rekey(phy->dev, vif, suspend);
	mt76_connac_mcu_set_arp_filter(phy->dev, vif, suspend);

	mt76_connac_mcu_set_suspend_mode(phy->dev, vif, suspend, 1, true);

	for (i = 0; i < wowlan->n_patterns; i++)
		mt76_connac_mcu_set_wow_pattern(phy->dev, vif, i, suspend,
						&wowlan->patterns[i]);
	mt76_connac_mcu_set_wow_ctrl(phy, vif, suspend, wowlan);
}
EXPORT_SYMBOL_GPL(mt76_connac_mcu_set_suspend_iter);
#endif /* CONFIG_PM */

u32 mt76_connac_mcu_reg_rr(struct mt76_dev *dev, u32 offset)
{
	struct {
		__le32 addr;
		__le32 val;
	} __packed req = {
		.addr = cpu_to_le32(offset),
	};

	return mt76_mcu_send_msg(dev, MCU_CMD_REG_READ, &req, sizeof(req),
				 true);
}
EXPORT_SYMBOL_GPL(mt76_connac_mcu_reg_rr);

void mt76_connac_mcu_reg_wr(struct mt76_dev *dev, u32 offset, u32 val)
{
	struct {
		__le32 addr;
		__le32 val;
	} __packed req = {
		.addr = cpu_to_le32(offset),
		.val = cpu_to_le32(val),
	};

	mt76_mcu_send_msg(dev, MCU_CMD_REG_WRITE, &req, sizeof(req), false);
}
EXPORT_SYMBOL_GPL(mt76_connac_mcu_reg_wr);

MODULE_AUTHOR("Lorenzo Bianconi <lorenzo@kernel.org>");
MODULE_LICENSE("Dual BSD/GPL");
	} __packed req = {
		.addr = cpu_to_le32(offset),
		.val = cpu_to_le32(val),
	};

	mt76_mcu_send_msg(dev, MCU_CMD_REG_WRITE, &req, sizeof(req), false);
}
EXPORT_SYMBOL_GPL(mt76_connac_mcu_reg_wr);

MODULE_AUTHOR("Lorenzo Bianconi <lorenzo@kernel.org>");
MODULE_LICENSE("Dual BSD/GPL");