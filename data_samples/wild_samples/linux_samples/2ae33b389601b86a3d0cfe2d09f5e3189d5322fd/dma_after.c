{
	unsigned ctlr;

	ctlr = EDMA_CTLR(channel);
	channel = EDMA_CHAN_SLOT(channel);

	if (channel >= edma_cc[ctlr]->num_channels)
		return;

	setup_dma_interrupt(channel, NULL, NULL);
	/* REVISIT should probably take out of shadow region 0 */

	memcpy_toio(edmacc_regs_base[ctlr] + PARM_OFFSET(channel),
			&dummy_paramset, PARM_SIZE);
	clear_bit(channel, edma_cc[ctlr]->edma_inuse);
}
EXPORT_SYMBOL(edma_free_channel);

/**
 * edma_alloc_slot - allocate DMA parameter RAM
 * @slot: specific slot to allocate; negative for "any unused slot"
 *
 * This allocates a parameter RAM slot, initializing it to hold a
 * dummy transfer.  Slots allocated using this routine have not been
 * mapped to a hardware DMA channel, and will normally be used by
 * linking to them from a slot associated with a DMA channel.
 *
 * Normal use is to pass EDMA_SLOT_ANY as the @slot, but specific
 * slots may be allocated on behalf of DSP firmware.
 *
 * Returns the number of the slot, else negative errno.
 */
int edma_alloc_slot(unsigned ctlr, int slot)
{
	if (!edma_cc[ctlr])
		return -EINVAL;

	if (slot >= 0)
		slot = EDMA_CHAN_SLOT(slot);

	if (slot < 0) {
		slot = edma_cc[ctlr]->num_channels;
		for (;;) {
			slot = find_next_zero_bit(edma_cc[ctlr]->edma_inuse,
					edma_cc[ctlr]->num_slots, slot);
			if (slot == edma_cc[ctlr]->num_slots)
				return -ENOMEM;
			if (!test_and_set_bit(slot, edma_cc[ctlr]->edma_inuse))
				break;
		}
	} else if (slot < edma_cc[ctlr]->num_channels ||
			slot >= edma_cc[ctlr]->num_slots) {
		return -EINVAL;
	} else if (test_and_set_bit(slot, edma_cc[ctlr]->edma_inuse)) {
		return -EBUSY;
	}

	memcpy_toio(edmacc_regs_base[ctlr] + PARM_OFFSET(slot),
			&dummy_paramset, PARM_SIZE);

	return EDMA_CTLR_CHAN(ctlr, slot);
}
EXPORT_SYMBOL(edma_alloc_slot);

/**
 * edma_free_slot - deallocate DMA parameter RAM
 * @slot: parameter RAM slot returned from edma_alloc_slot()
 *
 * This deallocates the parameter RAM slot allocated by edma_alloc_slot().
 * Callers are responsible for ensuring the slot is inactive, and will
 * not be activated.
 */
void edma_free_slot(unsigned slot)
{
	unsigned ctlr;

	ctlr = EDMA_CTLR(slot);
	slot = EDMA_CHAN_SLOT(slot);

	if (slot < edma_cc[ctlr]->num_channels ||
		slot >= edma_cc[ctlr]->num_slots)
		return;

	memcpy_toio(edmacc_regs_base[ctlr] + PARM_OFFSET(slot),
			&dummy_paramset, PARM_SIZE);
	clear_bit(slot, edma_cc[ctlr]->edma_inuse);
}
EXPORT_SYMBOL(edma_free_slot);


/**
 * edma_alloc_cont_slots- alloc contiguous parameter RAM slots
 * The API will return the starting point of a set of
 * contiguous parameter RAM slots that have been requested
 *
 * @id: can only be EDMA_CONT_PARAMS_ANY or EDMA_CONT_PARAMS_FIXED_EXACT
 * or EDMA_CONT_PARAMS_FIXED_NOT_EXACT
 * @count: number of contiguous Paramter RAM slots
 * @slot  - the start value of Parameter RAM slot that should be passed if id
 * is EDMA_CONT_PARAMS_FIXED_EXACT or EDMA_CONT_PARAMS_FIXED_NOT_EXACT
 *
 * If id is EDMA_CONT_PARAMS_ANY then the API starts looking for a set of
 * contiguous Parameter RAM slots from parameter RAM 64 in the case of
 * DaVinci SOCs and 32 in the case of DA8xx SOCs.
 *
 * If id is EDMA_CONT_PARAMS_FIXED_EXACT then the API starts looking for a
 * set of contiguous parameter RAM slots from the "slot" that is passed as an
 * argument to the API.
 *
 * If id is EDMA_CONT_PARAMS_FIXED_NOT_EXACT then the API initially tries
 * starts looking for a set of contiguous parameter RAMs from the "slot"
 * that is passed as an argument to the API. On failure the API will try to
 * find a set of contiguous Parameter RAM slots from the remaining Parameter
 * RAM slots
 */
int edma_alloc_cont_slots(unsigned ctlr, unsigned int id, int slot, int count)
{
	/*
	 * The start slot requested should be greater than
	 * the number of channels and lesser than the total number
	 * of slots
	 */
	if ((id != EDMA_CONT_PARAMS_ANY) &&
		(slot < edma_cc[ctlr]->num_channels ||
		slot >= edma_cc[ctlr]->num_slots))
		return -EINVAL;

	/*
	 * The number of parameter RAM slots requested cannot be less than 1
	 * and cannot be more than the number of slots minus the number of
	 * channels
	 */
	if (count < 1 || count >
		(edma_cc[ctlr]->num_slots - edma_cc[ctlr]->num_channels))
		return -EINVAL;

	switch (id) {
	case EDMA_CONT_PARAMS_ANY:
		return reserve_contiguous_slots(ctlr, id, count,
						 edma_cc[ctlr]->num_channels);
	case EDMA_CONT_PARAMS_FIXED_EXACT:
	case EDMA_CONT_PARAMS_FIXED_NOT_EXACT:
		return reserve_contiguous_slots(ctlr, id, count, slot);
	default:
		return -EINVAL;
	}

}
EXPORT_SYMBOL(edma_alloc_cont_slots);

/**
 * edma_free_cont_slots - deallocate DMA parameter RAM slots
 * @slot: first parameter RAM of a set of parameter RAM slots to be freed
 * @count: the number of contiguous parameter RAM slots to be freed
 *
 * This deallocates the parameter RAM slots allocated by
 * edma_alloc_cont_slots.
 * Callers/applications need to keep track of sets of contiguous
 * parameter RAM slots that have been allocated using the edma_alloc_cont_slots
 * API.
 * Callers are responsible for ensuring the slots are inactive, and will
 * not be activated.
 */
int edma_free_cont_slots(unsigned slot, int count)
{
	unsigned ctlr, slot_to_free;
	int i;

	ctlr = EDMA_CTLR(slot);
	slot = EDMA_CHAN_SLOT(slot);

	if (slot < edma_cc[ctlr]->num_channels ||
		slot >= edma_cc[ctlr]->num_slots ||
		count < 1)
		return -EINVAL;

	for (i = slot; i < slot + count; ++i) {
		ctlr = EDMA_CTLR(i);
		slot_to_free = EDMA_CHAN_SLOT(i);

		memcpy_toio(edmacc_regs_base[ctlr] + PARM_OFFSET(slot_to_free),
			&dummy_paramset, PARM_SIZE);
		clear_bit(slot_to_free, edma_cc[ctlr]->edma_inuse);
	}

	return 0;
}
EXPORT_SYMBOL(edma_free_cont_slots);

/*-----------------------------------------------------------------------*/

/* Parameter RAM operations (i) -- read/write partial slots */

/**
 * edma_set_src - set initial DMA source address in parameter RAM slot
 * @slot: parameter RAM slot being configured
 * @src_port: physical address of source (memory, controller FIFO, etc)
 * @addressMode: INCR, except in very rare cases
 * @fifoWidth: ignored unless @addressMode is FIFO, else specifies the
 *	width to use when addressing the fifo (e.g. W8BIT, W32BIT)
 *
 * Note that the source address is modified during the DMA transfer
 * according to edma_set_src_index().
 */
void edma_set_src(unsigned slot, dma_addr_t src_port,
				enum address_mode mode, enum fifo_width width)
{
	unsigned ctlr;

	ctlr = EDMA_CTLR(slot);
	slot = EDMA_CHAN_SLOT(slot);

	if (slot < edma_cc[ctlr]->num_slots) {
		unsigned int i = edma_parm_read(ctlr, PARM_OPT, slot);

		if (mode) {
			/* set SAM and program FWID */
			i = (i & ~(EDMA_FWID)) | (SAM | ((width & 0x7) << 8));
		} else {
			/* clear SAM */
			i &= ~SAM;
		}
		edma_parm_write(ctlr, PARM_OPT, slot, i);

		/* set the source port address
		   in source register of param structure */
		edma_parm_write(ctlr, PARM_SRC, slot, src_port);
	}
}
EXPORT_SYMBOL(edma_set_src);

/**
 * edma_set_dest - set initial DMA destination address in parameter RAM slot
 * @slot: parameter RAM slot being configured
 * @dest_port: physical address of destination (memory, controller FIFO, etc)
 * @addressMode: INCR, except in very rare cases
 * @fifoWidth: ignored unless @addressMode is FIFO, else specifies the
 *	width to use when addressing the fifo (e.g. W8BIT, W32BIT)
 *
 * Note that the destination address is modified during the DMA transfer
 * according to edma_set_dest_index().
 */
void edma_set_dest(unsigned slot, dma_addr_t dest_port,
				 enum address_mode mode, enum fifo_width width)
{
	unsigned ctlr;

	ctlr = EDMA_CTLR(slot);
	slot = EDMA_CHAN_SLOT(slot);

	if (slot < edma_cc[ctlr]->num_slots) {
		unsigned int i = edma_parm_read(ctlr, PARM_OPT, slot);

		if (mode) {
			/* set DAM and program FWID */
			i = (i & ~(EDMA_FWID)) | (DAM | ((width & 0x7) << 8));
		} else {
			/* clear DAM */
			i &= ~DAM;
		}
		edma_parm_write(ctlr, PARM_OPT, slot, i);
		/* set the destination port address
		   in dest register of param structure */
		edma_parm_write(ctlr, PARM_DST, slot, dest_port);
	}
}
EXPORT_SYMBOL(edma_set_dest);

/**
 * edma_get_position - returns the current transfer points
 * @slot: parameter RAM slot being examined
 * @src: pointer to source port position
 * @dst: pointer to destination port position
 *
 * Returns current source and destination addresses for a particular
 * parameter RAM slot.  Its channel should not be active when this is called.
 */
void edma_get_position(unsigned slot, dma_addr_t *src, dma_addr_t *dst)
{
	struct edmacc_param temp;
	unsigned ctlr;

	ctlr = EDMA_CTLR(slot);
	slot = EDMA_CHAN_SLOT(slot);

	edma_read_slot(EDMA_CTLR_CHAN(ctlr, slot), &temp);
	if (src != NULL)
		*src = temp.src;
	if (dst != NULL)
		*dst = temp.dst;
}
EXPORT_SYMBOL(edma_get_position);

/**
 * edma_set_src_index - configure DMA source address indexing
 * @slot: parameter RAM slot being configured
 * @src_bidx: byte offset between source arrays in a frame
 * @src_cidx: byte offset between source frames in a block
 *
 * Offsets are specified to support either contiguous or discontiguous
 * memory transfers, or repeated access to a hardware register, as needed.
 * When accessing hardware registers, both offsets are normally zero.
 */
void edma_set_src_index(unsigned slot, s16 src_bidx, s16 src_cidx)
{
	unsigned ctlr;

	ctlr = EDMA_CTLR(slot);
	slot = EDMA_CHAN_SLOT(slot);

	if (slot < edma_cc[ctlr]->num_slots) {
		edma_parm_modify(ctlr, PARM_SRC_DST_BIDX, slot,
				0xffff0000, src_bidx);
		edma_parm_modify(ctlr, PARM_SRC_DST_CIDX, slot,
				0xffff0000, src_cidx);
	}
}
EXPORT_SYMBOL(edma_set_src_index);

/**
 * edma_set_dest_index - configure DMA destination address indexing
 * @slot: parameter RAM slot being configured
 * @dest_bidx: byte offset between destination arrays in a frame
 * @dest_cidx: byte offset between destination frames in a block
 *
 * Offsets are specified to support either contiguous or discontiguous
 * memory transfers, or repeated access to a hardware register, as needed.
 * When accessing hardware registers, both offsets are normally zero.
 */
void edma_set_dest_index(unsigned slot, s16 dest_bidx, s16 dest_cidx)
{
	unsigned ctlr;

	ctlr = EDMA_CTLR(slot);
	slot = EDMA_CHAN_SLOT(slot);

	if (slot < edma_cc[ctlr]->num_slots) {
		edma_parm_modify(ctlr, PARM_SRC_DST_BIDX, slot,
				0x0000ffff, dest_bidx << 16);
		edma_parm_modify(ctlr, PARM_SRC_DST_CIDX, slot,
				0x0000ffff, dest_cidx << 16);
	}
}
EXPORT_SYMBOL(edma_set_dest_index);

/**
 * edma_set_transfer_params - configure DMA transfer parameters
 * @slot: parameter RAM slot being configured
 * @acnt: how many bytes per array (at least one)
 * @bcnt: how many arrays per frame (at least one)
 * @ccnt: how many frames per block (at least one)
 * @bcnt_rld: used only for A-Synchronized transfers; this specifies
 *	the value to reload into bcnt when it decrements to zero
 * @sync_mode: ASYNC or ABSYNC
 *
 * See the EDMA3 documentation to understand how to configure and link
 * transfers using the fields in PaRAM slots.  If you are not doing it
 * all at once with edma_write_slot(), you will use this routine
 * plus two calls each for source and destination, setting the initial
 * address and saying how to index that address.
 *
 * An example of an A-Synchronized transfer is a serial link using a
 * single word shift register.  In that case, @acnt would be equal to
 * that word size; the serial controller issues a DMA synchronization
 * event to transfer each word, and memory access by the DMA transfer
 * controller will be word-at-a-time.
 *
 * An example of an AB-Synchronized transfer is a device using a FIFO.
 * In that case, @acnt equals the FIFO width and @bcnt equals its depth.
 * The controller with the FIFO issues DMA synchronization events when
 * the FIFO threshold is reached, and the DMA transfer controller will
 * transfer one frame to (or from) the FIFO.  It will probably use
 * efficient burst modes to access memory.
 */
void edma_set_transfer_params(unsigned slot,
		u16 acnt, u16 bcnt, u16 ccnt,
		u16 bcnt_rld, enum sync_dimension sync_mode)
{
	unsigned ctlr;

	ctlr = EDMA_CTLR(slot);
	slot = EDMA_CHAN_SLOT(slot);

	if (slot < edma_cc[ctlr]->num_slots) {
		edma_parm_modify(ctlr, PARM_LINK_BCNTRLD, slot,
				0x0000ffff, bcnt_rld << 16);
		if (sync_mode == ASYNC)
			edma_parm_and(ctlr, PARM_OPT, slot, ~SYNCDIM);
		else
			edma_parm_or(ctlr, PARM_OPT, slot, SYNCDIM);
		/* Set the acount, bcount, ccount registers */
		edma_parm_write(ctlr, PARM_A_B_CNT, slot, (bcnt << 16) | acnt);
		edma_parm_write(ctlr, PARM_CCNT, slot, ccnt);
	}
}
EXPORT_SYMBOL(edma_set_transfer_params);

/**
 * edma_link - link one parameter RAM slot to another
 * @from: parameter RAM slot originating the link
 * @to: parameter RAM slot which is the link target
 *
 * The originating slot should not be part of any active DMA transfer.
 */
void edma_link(unsigned from, unsigned to)
{
	unsigned ctlr_from, ctlr_to;

	ctlr_from = EDMA_CTLR(from);
	from = EDMA_CHAN_SLOT(from);
	ctlr_to = EDMA_CTLR(to);
	to = EDMA_CHAN_SLOT(to);

	if (from >= edma_cc[ctlr_from]->num_slots)
		return;
	if (to >= edma_cc[ctlr_to]->num_slots)
		return;
	edma_parm_modify(ctlr_from, PARM_LINK_BCNTRLD, from, 0xffff0000,
				PARM_OFFSET(to));
}
EXPORT_SYMBOL(edma_link);

/**
 * edma_unlink - cut link from one parameter RAM slot
 * @from: parameter RAM slot originating the link
 *
 * The originating slot should not be part of any active DMA transfer.
 * Its link is set to 0xffff.
 */
void edma_unlink(unsigned from)
{
	unsigned ctlr;

	ctlr = EDMA_CTLR(from);
	from = EDMA_CHAN_SLOT(from);

	if (from >= edma_cc[ctlr]->num_slots)
		return;
	edma_parm_or(ctlr, PARM_LINK_BCNTRLD, from, 0xffff);
}
EXPORT_SYMBOL(edma_unlink);

/*-----------------------------------------------------------------------*/

/* Parameter RAM operations (ii) -- read/write whole parameter sets */

/**
 * edma_write_slot - write parameter RAM data for slot
 * @slot: number of parameter RAM slot being modified
 * @param: data to be written into parameter RAM slot
 *
 * Use this to assign all parameters of a transfer at once.  This
 * allows more efficient setup of transfers than issuing multiple
 * calls to set up those parameters in small pieces, and provides
 * complete control over all transfer options.
 */
void edma_write_slot(unsigned slot, const struct edmacc_param *param)
{
	unsigned ctlr;

	ctlr = EDMA_CTLR(slot);
	slot = EDMA_CHAN_SLOT(slot);

	if (slot >= edma_cc[ctlr]->num_slots)
		return;
	memcpy_toio(edmacc_regs_base[ctlr] + PARM_OFFSET(slot), param,
			PARM_SIZE);
}
EXPORT_SYMBOL(edma_write_slot);

/**
 * edma_read_slot - read parameter RAM data from slot
 * @slot: number of parameter RAM slot being copied
 * @param: where to store copy of parameter RAM data
 *
 * Use this to read data from a parameter RAM slot, perhaps to
 * save them as a template for later reuse.
 */
void edma_read_slot(unsigned slot, struct edmacc_param *param)
{
	unsigned ctlr;

	ctlr = EDMA_CTLR(slot);
	slot = EDMA_CHAN_SLOT(slot);

	if (slot >= edma_cc[ctlr]->num_slots)
		return;
	memcpy_fromio(param, edmacc_regs_base[ctlr] + PARM_OFFSET(slot),
			PARM_SIZE);
}
EXPORT_SYMBOL(edma_read_slot);

/*-----------------------------------------------------------------------*/

/* Various EDMA channel control operations */

/**
 * edma_pause - pause dma on a channel
 * @channel: on which edma_start() has been called
 *
 * This temporarily disables EDMA hardware events on the specified channel,
 * preventing them from triggering new transfers on its behalf
 */
void edma_pause(unsigned channel)
{
	unsigned ctlr;

	ctlr = EDMA_CTLR(channel);
	channel = EDMA_CHAN_SLOT(channel);

	if (channel < edma_cc[ctlr]->num_channels) {
		unsigned int mask = BIT(channel & 0x1f);

		edma_shadow0_write_array(ctlr, SH_EECR, channel >> 5, mask);
	}
}
EXPORT_SYMBOL(edma_pause);

/**
 * edma_resume - resumes dma on a paused channel
 * @channel: on which edma_pause() has been called
 *
 * This re-enables EDMA hardware events on the specified channel.
 */
void edma_resume(unsigned channel)
{
	unsigned ctlr;

	ctlr = EDMA_CTLR(channel);
	channel = EDMA_CHAN_SLOT(channel);

	if (channel < edma_cc[ctlr]->num_channels) {
		unsigned int mask = BIT(channel & 0x1f);

		edma_shadow0_write_array(ctlr, SH_EESR, channel >> 5, mask);
	}
}
EXPORT_SYMBOL(edma_resume);

/**
 * edma_start - start dma on a channel
 * @channel: channel being activated
 *
 * Channels with event associations will be triggered by their hardware
 * events, and channels without such associations will be triggered by
 * software.  (At this writing there is no interface for using software
 * triggers except with channels that don't support hardware triggers.)
 *
 * Returns zero on success, else negative errno.
 */
int edma_start(unsigned channel)
{
	unsigned ctlr;

	ctlr = EDMA_CTLR(channel);
	channel = EDMA_CHAN_SLOT(channel);

	if (channel < edma_cc[ctlr]->num_channels) {
		int j = channel >> 5;
		unsigned int mask = BIT(channel & 0x1f);

		/* EDMA channels without event association */
		if (test_bit(channel, edma_cc[ctlr]->edma_unused)) {
			pr_debug("EDMA: ESR%d %08x\n", j,
				edma_shadow0_read_array(ctlr, SH_ESR, j));
			edma_shadow0_write_array(ctlr, SH_ESR, j, mask);
			return 0;
		}

		/* EDMA channel with event association */
		pr_debug("EDMA: ER%d %08x\n", j,
			edma_shadow0_read_array(ctlr, SH_ER, j));
		/* Clear any pending event or error */
		edma_write_array(ctlr, EDMA_ECR, j, mask);
		edma_write_array(ctlr, EDMA_EMCR, j, mask);
		/* Clear any SER */
		edma_shadow0_write_array(ctlr, SH_SECR, j, mask);
		edma_shadow0_write_array(ctlr, SH_EESR, j, mask);
		pr_debug("EDMA: EER%d %08x\n", j,
			edma_shadow0_read_array(ctlr, SH_EER, j));
		return 0;
	}

	return -EINVAL;
}
EXPORT_SYMBOL(edma_start);

/**
 * edma_stop - stops dma on the channel passed
 * @channel: channel being deactivated
 *
 * When @lch is a channel, any active transfer is paused and
 * all pending hardware events are cleared.  The current transfer
 * may not be resumed, and the channel's Parameter RAM should be
 * reinitialized before being reused.
 */
void edma_stop(unsigned channel)
{
	unsigned ctlr;

	ctlr = EDMA_CTLR(channel);
	channel = EDMA_CHAN_SLOT(channel);

	if (channel < edma_cc[ctlr]->num_channels) {
		int j = channel >> 5;
		unsigned int mask = BIT(channel & 0x1f);

		edma_shadow0_write_array(ctlr, SH_EECR, j, mask);
		edma_shadow0_write_array(ctlr, SH_ECR, j, mask);
		edma_shadow0_write_array(ctlr, SH_SECR, j, mask);
		edma_write_array(ctlr, EDMA_EMCR, j, mask);

		pr_debug("EDMA: EER%d %08x\n", j,
				edma_shadow0_read_array(ctlr, SH_EER, j));

		/* REVISIT:  consider guarding against inappropriate event
		 * chaining by overwriting with dummy_paramset.
		 */
	}
}
EXPORT_SYMBOL(edma_stop);

/******************************************************************************
 *
 * It cleans ParamEntry qand bring back EDMA to initial state if media has
 * been removed before EDMA has finished.It is usedful for removable media.
 * Arguments:
 *      ch_no     - channel no
 *
 * Return: zero on success, or corresponding error no on failure
 *
 * FIXME this should not be needed ... edma_stop() should suffice.
 *
 *****************************************************************************/

void edma_clean_channel(unsigned channel)
{
	unsigned ctlr;

	ctlr = EDMA_CTLR(channel);
	channel = EDMA_CHAN_SLOT(channel);

	if (channel < edma_cc[ctlr]->num_channels) {
		int j = (channel >> 5);
		unsigned int mask = BIT(channel & 0x1f);

		pr_debug("EDMA: EMR%d %08x\n", j,
				edma_read_array(ctlr, EDMA_EMR, j));
		edma_shadow0_write_array(ctlr, SH_ECR, j, mask);
		/* Clear the corresponding EMR bits */
		edma_write_array(ctlr, EDMA_EMCR, j, mask);
		/* Clear any SER */
		edma_shadow0_write_array(ctlr, SH_SECR, j, mask);
		edma_write(ctlr, EDMA_CCERRCLR, BIT(16) | BIT(1) | BIT(0));
	}
}
EXPORT_SYMBOL(edma_clean_channel);

/*
 * edma_clear_event - clear an outstanding event on the DMA channel
 * Arguments:
 *	channel - channel number
 */
void edma_clear_event(unsigned channel)
{
	unsigned ctlr;

	ctlr = EDMA_CTLR(channel);
	channel = EDMA_CHAN_SLOT(channel);

	if (channel >= edma_cc[ctlr]->num_channels)
		return;
	if (channel < 32)
		edma_write(ctlr, EDMA_ECR, BIT(channel));
	else
		edma_write(ctlr, EDMA_ECRH, BIT(channel - 32));
}
EXPORT_SYMBOL(edma_clear_event);

/*-----------------------------------------------------------------------*/

static int __init edma_probe(struct platform_device *pdev)
{
	struct edma_soc_info	**info = pdev->dev.platform_data;
	const s8		(*queue_priority_mapping)[2];
	const s8		(*queue_tc_mapping)[2];
	int			i, j, off, ln, found = 0;
	int			status = -1;
	const s16		(*rsv_chans)[2];
	const s16		(*rsv_slots)[2];
	int			irq[EDMA_MAX_CC] = {0, 0};
	int			err_irq[EDMA_MAX_CC] = {0, 0};
	struct resource		*r[EDMA_MAX_CC] = {NULL};
	resource_size_t		len[EDMA_MAX_CC];
	char			res_name[10];
	char			irq_name[10];

	if (!info)
		return -ENODEV;

	for (j = 0; j < EDMA_MAX_CC; j++) {
		sprintf(res_name, "edma_cc%d", j);
		r[j] = platform_get_resource_byname(pdev, IORESOURCE_MEM,
						res_name);
		if (!r[j] || !info[j]) {
			if (found)
				break;
			else
				return -ENODEV;
		} else {
			found = 1;
		}

		len[j] = resource_size(r[j]);

		r[j] = request_mem_region(r[j]->start, len[j],
			dev_name(&pdev->dev));
		if (!r[j]) {
			status = -EBUSY;
			goto fail1;
		}

		edmacc_regs_base[j] = ioremap(r[j]->start, len[j]);
		if (!edmacc_regs_base[j]) {
			status = -EBUSY;
			goto fail1;
		}

		edma_cc[j] = kzalloc(sizeof(struct edma), GFP_KERNEL);
		if (!edma_cc[j]) {
			status = -ENOMEM;
			goto fail1;
		}

		edma_cc[j]->num_channels = min_t(unsigned, info[j]->n_channel,
							EDMA_MAX_DMACH);
		edma_cc[j]->num_slots = min_t(unsigned, info[j]->n_slot,
							EDMA_MAX_PARAMENTRY);
		edma_cc[j]->num_cc = min_t(unsigned, info[j]->n_cc,
							EDMA_MAX_CC);

		edma_cc[j]->default_queue = info[j]->default_queue;

		dev_dbg(&pdev->dev, "DMA REG BASE ADDR=%p\n",
			edmacc_regs_base[j]);

		for (i = 0; i < edma_cc[j]->num_slots; i++)
			memcpy_toio(edmacc_regs_base[j] + PARM_OFFSET(i),
					&dummy_paramset, PARM_SIZE);

		/* Mark all channels as unused */
		memset(edma_cc[j]->edma_unused, 0xff,
			sizeof(edma_cc[j]->edma_unused));

		if (info[j]->rsv) {

			/* Clear the reserved channels in unused list */
			rsv_chans = info[j]->rsv->rsv_chans;
			if (rsv_chans) {
				for (i = 0; rsv_chans[i][0] != -1; i++) {
					off = rsv_chans[i][0];
					ln = rsv_chans[i][1];
					clear_bits(off, ln,
						edma_cc[j]->edma_unused);
				}
			}

			/* Set the reserved slots in inuse list */
			rsv_slots = info[j]->rsv->rsv_slots;
			if (rsv_slots) {
				for (i = 0; rsv_slots[i][0] != -1; i++) {
					off = rsv_slots[i][0];
					ln = rsv_slots[i][1];
					set_bits(off, ln,
						edma_cc[j]->edma_inuse);
				}
			}
		}

		sprintf(irq_name, "edma%d", j);
		irq[j] = platform_get_irq_byname(pdev, irq_name);
		edma_cc[j]->irq_res_start = irq[j];
		status = request_irq(irq[j], dma_irq_handler, 0, "edma",
					&pdev->dev);
		if (status < 0) {
			dev_dbg(&pdev->dev, "request_irq %d failed --> %d\n",
				irq[j], status);
			goto fail;
		}

		sprintf(irq_name, "edma%d_err", j);
		err_irq[j] = platform_get_irq_byname(pdev, irq_name);
		edma_cc[j]->irq_res_end = err_irq[j];
		status = request_irq(err_irq[j], dma_ccerr_handler, 0,
					"edma_error", &pdev->dev);
		if (status < 0) {
			dev_dbg(&pdev->dev, "request_irq %d failed --> %d\n",
				err_irq[j], status);
			goto fail;
		}

		for (i = 0; i < edma_cc[j]->num_channels; i++)
			map_dmach_queue(j, i, info[j]->default_queue);

		queue_tc_mapping = info[j]->queue_tc_mapping;
		queue_priority_mapping = info[j]->queue_priority_mapping;

		/* Event queue to TC mapping */
		for (i = 0; queue_tc_mapping[i][0] != -1; i++)
			map_queue_tc(j, queue_tc_mapping[i][0],
					queue_tc_mapping[i][1]);

		/* Event queue priority mapping */
		for (i = 0; queue_priority_mapping[i][0] != -1; i++)
			assign_priority_to_queue(j,
						queue_priority_mapping[i][0],
						queue_priority_mapping[i][1]);

		/* Map the channel to param entry if channel mapping logic
		 * exist
		 */
		if (edma_read(j, EDMA_CCCFG) & CHMAP_EXIST)
			map_dmach_param(j);

		for (i = 0; i < info[j]->n_region; i++) {
			edma_write_array2(j, EDMA_DRAE, i, 0, 0x0);
			edma_write_array2(j, EDMA_DRAE, i, 1, 0x0);
			edma_write_array(j, EDMA_QRAE, i, 0x0);
		}
		arch_num_cc++;
	}

	if (tc_errs_handled) {
		status = request_irq(IRQ_TCERRINT0, dma_tc0err_handler, 0,
					"edma_tc0", &pdev->dev);
		if (status < 0) {
			dev_dbg(&pdev->dev, "request_irq %d failed --> %d\n",
				IRQ_TCERRINT0, status);
			return status;
		}
		status = request_irq(IRQ_TCERRINT, dma_tc1err_handler, 0,
					"edma_tc1", &pdev->dev);
		if (status < 0) {
			dev_dbg(&pdev->dev, "request_irq %d --> %d\n",
				IRQ_TCERRINT, status);
			return status;
		}
	}

	return 0;

fail:
	for (i = 0; i < EDMA_MAX_CC; i++) {
		if (err_irq[i])
			free_irq(err_irq[i], &pdev->dev);
		if (irq[i])
			free_irq(irq[i], &pdev->dev);
	}
fail1:
	for (i = 0; i < EDMA_MAX_CC; i++) {
		if (r[i])
			release_mem_region(r[i]->start, len[i]);
		if (edmacc_regs_base[i])
			iounmap(edmacc_regs_base[i]);
		kfree(edma_cc[i]);
	}
	return status;
}


static struct platform_driver edma_driver = {
	.driver.name	= "edma",
};

static int __init edma_init(void)
{
	return platform_driver_probe(&edma_driver, edma_probe);
}
arch_initcall(edma_init);
