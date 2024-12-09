 */
int edma_alloc_slot(unsigned ctlr, int slot)
{
	if (slot >= 0)
		slot = EDMA_CHAN_SLOT(slot);

	if (slot < 0) {