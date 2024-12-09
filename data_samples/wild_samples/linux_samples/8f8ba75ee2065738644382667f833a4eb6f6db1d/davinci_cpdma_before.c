	} else {
		chan->hdp	= ctlr->params.txhdp + offset;
		chan->cp	= ctlr->params.txcp + offset;
		chan->int_set	= CPDMA_TXINTMASKSET;
		chan->int_clear	= CPDMA_TXINTMASKCLEAR;
		chan->td	= CPDMA_TXTEARDOWN;
		chan->dir	= DMA_TO_DEVICE;
	}
	chan->mask = BIT(chan_linear(chan));

	spin_lock_init(&chan->lock);

	ctlr->channels[chan_num] = chan;
	spin_unlock_irqrestore(&ctlr->lock, flags);
	return chan;

err_chan_busy:
	spin_unlock_irqrestore(&ctlr->lock, flags);
	kfree(chan);
err_chan_alloc:
	return ERR_PTR(ret);
}
EXPORT_SYMBOL_GPL(cpdma_chan_create);

int cpdma_chan_destroy(struct cpdma_chan *chan)
{
	struct cpdma_ctlr *ctlr = chan->ctlr;
	unsigned long flags;

	if (!chan)
		return -EINVAL;

	spin_lock_irqsave(&ctlr->lock, flags);
	if (chan->state != CPDMA_STATE_IDLE)
		cpdma_chan_stop(chan);
	ctlr->channels[chan->chan_num] = NULL;
	spin_unlock_irqrestore(&ctlr->lock, flags);
	kfree(chan);
	return 0;
}