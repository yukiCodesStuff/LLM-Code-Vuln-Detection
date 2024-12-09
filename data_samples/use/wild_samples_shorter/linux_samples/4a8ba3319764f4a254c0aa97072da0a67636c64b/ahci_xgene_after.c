 * xgene_ahci_qc_issue - Issue commands to the device
 * @qc: Command to issue
 *
 * Due to Hardware errata for IDENTIFY DEVICE command and PACKET
 * command of ATAPI protocol set, the controller cannot clear the BSY bit
 * after receiving the PIO setup FIS. This results in the DMA state machine
 * going into the CMFatalErrorUpdate state and locks up. By restarting the
 * DMA engine, it removes the controller out of lock up state.
 */
static unsigned int xgene_ahci_qc_issue(struct ata_queued_cmd *qc)
{
	struct ata_port *ap = qc->ap;
	struct xgene_ahci_context *ctx = hpriv->plat_data;
	int rc = 0;

	if (unlikely((ctx->last_cmd[ap->port_no] == ATA_CMD_ID_ATA) ||
	    (ctx->last_cmd[ap->port_no] == ATA_CMD_PACKET)))
		xgene_ahci_restart_engine(ap);

	rc = ahci_qc_issue(qc);

	 *
	 * Clear reserved bit 8 (DEVSLP bit) as we don't support DEVSLP
	 */
	id[ATA_ID_FEATURE_SUPP] &= cpu_to_le16(~(1 << 8));

	return 0;
}
