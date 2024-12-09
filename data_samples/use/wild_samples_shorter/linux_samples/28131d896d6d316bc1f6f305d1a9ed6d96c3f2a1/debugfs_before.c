			 mt7915_radar_trigger, "%lld\n");

static int
mt7915_fw_debug_set(void *data, u64 val)
{
	struct mt7915_dev *dev = data;
	enum {
		DEBUG_TXCMD = 62,
		DEBUG_SPL,
		DEBUG_RPT_RX,
	} debug;

	dev->fw_debug = !!val;

	mt7915_mcu_fw_log_2_host(dev, dev->fw_debug ? 2 : 0);

	for (debug = DEBUG_TXCMD; debug <= DEBUG_RPT_RX; debug++)
		mt7915_mcu_fw_dbg_ctrl(dev, debug, dev->fw_debug);

	return 0;
}

static int
mt7915_fw_debug_get(void *data, u64 *val)
{
	struct mt7915_dev *dev = data;

	*val = dev->fw_debug;

	return 0;
}

DEFINE_DEBUGFS_ATTRIBUTE(fops_fw_debug, mt7915_fw_debug_get,
			 mt7915_fw_debug_set, "%lld\n");

static void
mt7915_ampdu_stat_read_phy(struct mt7915_phy *phy,
			   struct seq_file *file)
	debugfs_create_file("xmit-queues", 0400, dir, phy,
			    &mt7915_xmit_queues_fops);
	debugfs_create_file("tx_stats", 0400, dir, phy, &mt7915_tx_stats_fops);
	debugfs_create_file("fw_debug", 0600, dir, dev, &fops_fw_debug);
	debugfs_create_file("implicit_txbf", 0600, dir, dev,
			    &fops_implicit_txbf);
	debugfs_create_file("txpower_sku", 0400, dir, phy,
			    &mt7915_rate_txpower_fops);
#ifdef CONFIG_MAC80211_DEBUGFS
/** per-station debugfs **/

static int mt7915_sta_fixed_rate_set(void *data, u64 rate)
{
	struct ieee80211_sta *sta = data;
	struct mt7915_sta *msta = (struct mt7915_sta *)sta->drv_priv;

	/* usage: <he ltf> <tx mode> <ldpc> <stbc> <bw> <gi> <nss> <mcs>
	 * <tx mode>: see enum mt76_phy_type
	 */
	return mt7915_mcu_set_fixed_rate(msta->vif->phy->dev, sta, rate);
}

DEFINE_DEBUGFS_ATTRIBUTE(fops_fixed_rate, NULL,
			 mt7915_sta_fixed_rate_set, "%llx\n");

static int
mt7915_queues_show(struct seq_file *s, void *data)
{