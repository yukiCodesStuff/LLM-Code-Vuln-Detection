	mt76_connac_mcu_set_wow_ctrl(phy, vif, suspend, wowlan);
}
EXPORT_SYMBOL_GPL(mt76_connac_mcu_set_suspend_iter);

u32 mt76_connac_mcu_reg_rr(struct mt76_dev *dev, u32 offset)
{
	struct {
	mt76_mcu_send_msg(dev, MCU_CMD_REG_WRITE, &req, sizeof(req), false);
}
EXPORT_SYMBOL_GPL(mt76_connac_mcu_reg_wr);
#endif /* CONFIG_PM */

MODULE_AUTHOR("Lorenzo Bianconi <lorenzo@kernel.org>");
MODULE_LICENSE("Dual BSD/GPL");