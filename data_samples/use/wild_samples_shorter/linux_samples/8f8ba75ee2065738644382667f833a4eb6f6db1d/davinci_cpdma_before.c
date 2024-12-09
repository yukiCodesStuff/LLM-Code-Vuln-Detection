
int cpdma_chan_destroy(struct cpdma_chan *chan)
{
	struct cpdma_ctlr *ctlr = chan->ctlr;
	unsigned long flags;

	if (!chan)
		return -EINVAL;

	spin_lock_irqsave(&ctlr->lock, flags);
	if (chan->state != CPDMA_STATE_IDLE)
		cpdma_chan_stop(chan);