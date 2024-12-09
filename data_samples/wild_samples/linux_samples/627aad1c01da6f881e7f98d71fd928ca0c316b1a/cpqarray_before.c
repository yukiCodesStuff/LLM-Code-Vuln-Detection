	{
		
		ida_pci_info_struct pciinfo;

		if (!arg) return -EINVAL;
		pciinfo.bus = host->pci_dev->bus->number;
		pciinfo.dev_fn = host->pci_dev->devfn;
		pciinfo.board_id = host->board_id;
		if(copy_to_user((void __user *) arg, &pciinfo,  
			sizeof( ida_pci_info_struct)))
				return -EFAULT;
		return(0);
	}	

	default:
		return -EINVAL;
	}
		
}

static int ida_ioctl(struct block_device *bdev, fmode_t mode,
			     unsigned int cmd, unsigned long param)
{
	int ret;

	mutex_lock(&cpqarray_mutex);
	ret = ida_locked_ioctl(bdev, mode, cmd, param);
	mutex_unlock(&cpqarray_mutex);

	return ret;
}

/*
 * ida_ctlr_ioctl is for passing commands to the controller from userspace.
 * The command block (io) has already been copied to kernel space for us,
 * however, any elements in the sglist need to be copied to kernel space
 * or copied back to userspace.
 *
 * Only root may perform a controller passthru command, however I'm not doing
 * any serious sanity checking on the arguments.  Doing an IDA_WRITE_MEDIA and
 * putting a 64M buffer in the sglist is probably a *bad* idea.
 */
static int ida_ctlr_ioctl(ctlr_info_t *h, int dsk, ida_ioctl_t *io)
{
	int ctlr = h->ctlr;
	cmdlist_t *c;
	void *p = NULL;
	unsigned long flags;
	int error;

	if ((c = cmd_alloc(h, 0)) == NULL)
		return -ENOMEM;
	c->ctlr = ctlr;
	c->hdr.unit = (io->unit & UNITVALID) ? (io->unit & ~UNITVALID) : dsk;
	c->hdr.size = sizeof(rblk_t) >> 2;
	c->size += sizeof(rblk_t);

	c->req.hdr.cmd = io->cmd;
	c->req.hdr.blk = io->blk;
	c->req.hdr.blk_cnt = io->blk_cnt;
	c->type = CMD_IOCTL_PEND;

	/* Pre submit processing */
	switch(io->cmd) {
	case PASSTHRU_A:
		p = memdup_user(io->sg[0].addr, io->sg[0].size);
		if (IS_ERR(p)) {
			error = PTR_ERR(p);
			cmd_free(h, c, 0);
			return error;
		}
		c->req.hdr.blk = pci_map_single(h->pci_dev, &(io->c), 
				sizeof(ida_ioctl_t), 
				PCI_DMA_BIDIRECTIONAL);
		c->req.sg[0].size = io->sg[0].size;
		c->req.sg[0].addr = pci_map_single(h->pci_dev, p, 
			c->req.sg[0].size, PCI_DMA_BIDIRECTIONAL);
		c->req.hdr.sg_cnt = 1;
		break;
	case IDA_READ:
	case READ_FLASH_ROM:
	case SENSE_CONTROLLER_PERFORMANCE:
		p = kmalloc(io->sg[0].size, GFP_KERNEL);
		if (!p) 
		{ 
                        error = -ENOMEM; 
                        cmd_free(h, c, 0);
                        return(error);
                }

		c->req.sg[0].size = io->sg[0].size;
		c->req.sg[0].addr = pci_map_single(h->pci_dev, p, 
			c->req.sg[0].size, PCI_DMA_BIDIRECTIONAL); 
		c->req.hdr.sg_cnt = 1;
		break;
	case IDA_WRITE:
	case IDA_WRITE_MEDIA:
	case DIAG_PASS_THRU:
	case COLLECT_BUFFER:
	case WRITE_FLASH_ROM:
		p = memdup_user(io->sg[0].addr, io->sg[0].size);
		if (IS_ERR(p)) {
			error = PTR_ERR(p);
			cmd_free(h, c, 0);
			return error;
                }
		c->req.sg[0].size = io->sg[0].size;
		c->req.sg[0].addr = pci_map_single(h->pci_dev, p, 
			c->req.sg[0].size, PCI_DMA_BIDIRECTIONAL); 
		c->req.hdr.sg_cnt = 1;
		break;
	default:
		c->req.sg[0].size = sizeof(io->c);
		c->req.sg[0].addr = pci_map_single(h->pci_dev,&io->c, 
			c->req.sg[0].size, PCI_DMA_BIDIRECTIONAL);
		c->req.hdr.sg_cnt = 1;
	}
	
	/* Put the request on the tail of the request queue */
	spin_lock_irqsave(IDA_LOCK(ctlr), flags);
	addQ(&h->reqQ, c);
	h->Qdepth++;
	start_io(h);
	spin_unlock_irqrestore(IDA_LOCK(ctlr), flags);

	/* Wait for completion */
	while(c->type != CMD_IOCTL_DONE)
		schedule();

	/* Unmap the DMA  */
	pci_unmap_single(h->pci_dev, c->req.sg[0].addr, c->req.sg[0].size, 
		PCI_DMA_BIDIRECTIONAL);
	/* Post submit processing */
	switch(io->cmd) {
	case PASSTHRU_A:
		pci_unmap_single(h->pci_dev, c->req.hdr.blk,
                                sizeof(ida_ioctl_t),
                                PCI_DMA_BIDIRECTIONAL);
	case IDA_READ:
	case DIAG_PASS_THRU:
	case SENSE_CONTROLLER_PERFORMANCE:
	case READ_FLASH_ROM:
		if (copy_to_user(io->sg[0].addr, p, io->sg[0].size)) {
			kfree(p);
			return -EFAULT;
		}
		/* fall through and free p */
	case IDA_WRITE:
	case IDA_WRITE_MEDIA:
	case COLLECT_BUFFER:
	case WRITE_FLASH_ROM:
		kfree(p);
		break;
	default:;
		/* Nothing to do */
	}

	io->rcode = c->req.hdr.rcode;
	cmd_free(h, c, 0);
	return(0);
}

/*
 * Commands are pre-allocated in a large block.  Here we use a simple bitmap
 * scheme to suballocte them to the driver.  Operations that are not time
 * critical (and can wait for kmalloc and possibly sleep) can pass in NULL
 * as the first argument to get a new command.
 */
static cmdlist_t * cmd_alloc(ctlr_info_t *h, int get_from_pool)
{
	cmdlist_t * c;
	int i;
	dma_addr_t cmd_dhandle;

	if (!get_from_pool) {
		c = (cmdlist_t*)pci_alloc_consistent(h->pci_dev, 
			sizeof(cmdlist_t), &cmd_dhandle);
		if(c==NULL)
			return NULL;
	} else {
		do {
			i = find_first_zero_bit(h->cmd_pool_bits, NR_CMDS);
			if (i == NR_CMDS)
				return NULL;
		} while(test_and_set_bit(i&(BITS_PER_LONG-1), h->cmd_pool_bits+(i/BITS_PER_LONG)) != 0);
		c = h->cmd_pool + i;
		cmd_dhandle = h->cmd_pool_dhandle + i*sizeof(cmdlist_t);
		h->nr_allocs++;
	}