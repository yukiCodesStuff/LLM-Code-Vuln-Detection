	for (;;) {
		/*
		 * figure out where we are (as an index) and where
		 * the hardware is (also as an index)
		 *
		 * This could be done faster if (for example) the
		 * descriptor table was page-aligned and contiguous in
		 * both virtual and physical memory -- you could then
		 * just compare the low-order bits of the virtual address
		 * (sbdma_remptr) and the physical address (sbdma_curdscr CSR)
		 */

		curidx = d->sbdma_remptr - d->sbdma_dscrtable;

		/*
		 * If they're the same, that means we've processed all
		 * of the descriptors up to (but not including) the one that
		 * the hardware is working on right now.
		 */

		if (curidx == hwidx)
			break;

		/*
		 * Otherwise, get the packet's sk_buff ptr back
		 */

		dsc = &(d->sbdma_dscrtable[curidx]);
		sb = d->sbdma_ctxtable[curidx];
		d->sbdma_ctxtable[curidx] = NULL;

		/*
		 * Stats
		 */

		dev->stats.tx_bytes += sb->len;
		dev->stats.tx_packets++;

		/*
		 * for transmits, we just free buffers.
		 */

		dev_kfree_skb_irq(sb);

		/*
		 * .. and advance to the next buffer.
		 */

		d->sbdma_remptr = SBDMA_NEXTBUF(d,sbdma_remptr);

		packets_handled++;

	}

	/*
	 * Decide if we should wake up the protocol or not.
	 * Other drivers seem to do this when we reach a low
	 * watermark on the transmit queue.
	 */

	if (packets_handled)
		netif_wake_queue(d->sbdma_eth->sbm_dev);

end_unlock:
	spin_unlock_irqrestore(&(sc->sbm_lock), flags);

}



/**********************************************************************
 *  SBMAC_INITCTX(s)
 *
 *  Initialize an Ethernet context structure - this is called
 *  once per MAC on the 1250.  Memory is allocated here, so don't
 *  call it again from inside the ioctl routines that bring the
 *  interface up/down
 *
 *  Input parameters:
 *  	   s - sbmac context structure
 *
 *  Return value:
 *  	   0
 ********************************************************************* */

static int sbmac_initctx(struct sbmac_softc *s)
{

	/*
	 * figure out the addresses of some ports
	 */

	s->sbm_macenable = s->sbm_base + R_MAC_ENABLE;
	s->sbm_maccfg    = s->sbm_base + R_MAC_CFG;
	s->sbm_fifocfg   = s->sbm_base + R_MAC_THRSH_CFG;
	s->sbm_framecfg  = s->sbm_base + R_MAC_FRAMECFG;
	s->sbm_rxfilter  = s->sbm_base + R_MAC_ADFILTER_CFG;
	s->sbm_isr       = s->sbm_base + R_MAC_STATUS;
	s->sbm_imr       = s->sbm_base + R_MAC_INT_MASK;
	s->sbm_mdio      = s->sbm_base + R_MAC_MDIO;

	/*
	 * Initialize the DMA channels.  Right now, only one per MAC is used
	 * Note: Only do this _once_, as it allocates memory from the kernel!
	 */

	sbdma_initctx(&(s->sbm_txdma),s,0,DMA_TX,SBMAC_MAX_TXDESCR);
	sbdma_initctx(&(s->sbm_rxdma),s,0,DMA_RX,SBMAC_MAX_RXDESCR);

	/*
	 * initial state is OFF
	 */

	s->sbm_state = sbmac_state_off;

	return 0;
}


static void sbdma_uninitctx(struct sbmacdma *d)
{
	kfree(d->sbdma_dscrtable_unaligned);
	d->sbdma_dscrtable_unaligned = d->sbdma_dscrtable = NULL;

	kfree(d->sbdma_ctxtable);
	d->sbdma_ctxtable = NULL;
}


static void sbmac_uninitctx(struct sbmac_softc *sc)
{
	sbdma_uninitctx(&(sc->sbm_txdma));
	sbdma_uninitctx(&(sc->sbm_rxdma));
}


/**********************************************************************
 *  SBMAC_CHANNEL_START(s)
 *
 *  Start packet processing on this MAC.
 *
 *  Input parameters:
 *  	   s - sbmac structure
 *
 *  Return value:
 *  	   nothing
 ********************************************************************* */

static void sbmac_channel_start(struct sbmac_softc *s)
{
	uint64_t reg;
	void __iomem *port;
	uint64_t cfg,fifo,framecfg;
	int idx, th_value;

	/*
	 * Don't do this if running
	 */

	if (s->sbm_state == sbmac_state_on)
		return;

	/*
	 * Bring the controller out of reset, but leave it off.
	 */

	__raw_writeq(0, s->sbm_macenable);

	/*
	 * Ignore all received packets
	 */

	__raw_writeq(0, s->sbm_rxfilter);

	/*
	 * Calculate values for various control registers.
	 */

	cfg = M_MAC_RETRY_EN |
		M_MAC_TX_HOLD_SOP_EN |
		V_MAC_TX_PAUSE_CNT_16K |
		M_MAC_AP_STAT_EN |
		M_MAC_FAST_SYNC |
		M_MAC_SS_EN |
		0;

	/*
	 * Be sure that RD_THRSH+WR_THRSH <= 32 for pass1 pars
	 * and make sure that RD_THRSH + WR_THRSH <=128 for pass2 and above
	 * Use a larger RD_THRSH for gigabit
	 */
	if (soc_type == K_SYS_SOC_TYPE_BCM1250 && periph_rev < 2)
		th_value = 28;
	else
		th_value = 64;

	fifo = V_MAC_TX_WR_THRSH(4) |	/* Must be '4' or '8' */
		((s->sbm_speed == sbmac_speed_1000)
		 ? V_MAC_TX_RD_THRSH(th_value) : V_MAC_TX_RD_THRSH(4)) |
		V_MAC_TX_RL_THRSH(4) |
		V_MAC_RX_PL_THRSH(4) |
		V_MAC_RX_RD_THRSH(4) |	/* Must be '4' */
		V_MAC_RX_RL_THRSH(8) |
		0;

	framecfg = V_MAC_MIN_FRAMESZ_DEFAULT |
		V_MAC_MAX_FRAMESZ_DEFAULT |
		V_MAC_BACKOFF_SEL(1);

	/*
	 * Clear out the hash address map
	 */

	port = s->sbm_base + R_MAC_HASH_BASE;
	for (idx = 0; idx < MAC_HASH_COUNT; idx++) {
		__raw_writeq(0, port);
		port += sizeof(uint64_t);
	}

	/*
	 * Clear out the exact-match table
	 */

	port = s->sbm_base + R_MAC_ADDR_BASE;
	for (idx = 0; idx < MAC_ADDR_COUNT; idx++) {
		__raw_writeq(0, port);
		port += sizeof(uint64_t);
	}

	/*
	 * Clear out the DMA Channel mapping table registers
	 */

	port = s->sbm_base + R_MAC_CHUP0_BASE;
	for (idx = 0; idx < MAC_CHMAP_COUNT; idx++) {
		__raw_writeq(0, port);
		port += sizeof(uint64_t);
	}


	port = s->sbm_base + R_MAC_CHLO0_BASE;
	for (idx = 0; idx < MAC_CHMAP_COUNT; idx++) {
		__raw_writeq(0, port);
		port += sizeof(uint64_t);
	}

	/*
	 * Program the hardware address.  It goes into the hardware-address
	 * register as well as the first filter register.
	 */

	reg = sbmac_addr2reg(s->sbm_hwaddr);

	port = s->sbm_base + R_MAC_ADDR_BASE;
	__raw_writeq(reg, port);
	port = s->sbm_base + R_MAC_ETHERNET_ADDR;

	__raw_writeq(reg, port);

	/*
	 * Set the receive filter for no packets, and write values
	 * to the various config registers
	 */

	__raw_writeq(0, s->sbm_rxfilter);
	__raw_writeq(0, s->sbm_imr);
	__raw_writeq(framecfg, s->sbm_framecfg);
	__raw_writeq(fifo, s->sbm_fifocfg);
	__raw_writeq(cfg, s->sbm_maccfg);

	/*
	 * Initialize DMA channels (rings should be ok now)
	 */

	sbdma_channel_start(&(s->sbm_rxdma), DMA_RX);
	sbdma_channel_start(&(s->sbm_txdma), DMA_TX);

	/*
	 * Configure the speed, duplex, and flow control
	 */

	sbmac_set_speed(s,s->sbm_speed);
	sbmac_set_duplex(s,s->sbm_duplex,s->sbm_fc);

	/*
	 * Fill the receive ring
	 */

	sbdma_fillring(s, &(s->sbm_rxdma));

	/*
	 * Turn on the rest of the bits in the enable register
	 */

#if defined(CONFIG_SIBYTE_BCM1x55) || defined(CONFIG_SIBYTE_BCM1x80)
	__raw_writeq(M_MAC_RXDMA_EN0 |
		       M_MAC_TXDMA_EN0, s->sbm_macenable);
#elif defined(CONFIG_SIBYTE_SB1250) || defined(CONFIG_SIBYTE_BCM112X)
	__raw_writeq(M_MAC_RXDMA_EN0 |
		       M_MAC_TXDMA_EN0 |
		       M_MAC_RX_ENABLE |
		       M_MAC_TX_ENABLE, s->sbm_macenable);
#else
#error invalid SiByte MAC configuration
#endif

#ifdef CONFIG_SBMAC_COALESCE
	__raw_writeq(((M_MAC_INT_EOP_COUNT | M_MAC_INT_EOP_TIMER) << S_MAC_TX_CH0) |
		       ((M_MAC_INT_EOP_COUNT | M_MAC_INT_EOP_TIMER) << S_MAC_RX_CH0), s->sbm_imr);
#else
	__raw_writeq((M_MAC_INT_CHANNEL << S_MAC_TX_CH0) |
		       (M_MAC_INT_CHANNEL << S_MAC_RX_CH0), s->sbm_imr);
#endif

	/*
	 * Enable receiving unicasts and broadcasts
	 */

	__raw_writeq(M_MAC_UCAST_EN | M_MAC_BCAST_EN, s->sbm_rxfilter);

	/*
	 * we're running now.
	 */

	s->sbm_state = sbmac_state_on;

	/*
	 * Program multicast addresses
	 */

	sbmac_setmulti(s);

	/*
	 * If channel was in promiscuous mode before, turn that on
	 */

	if (s->sbm_devflags & IFF_PROMISC) {
		sbmac_promiscuous_mode(s,1);
	}

}


/**********************************************************************
 *  SBMAC_CHANNEL_STOP(s)
 *
 *  Stop packet processing on this MAC.
 *
 *  Input parameters:
 *  	   s - sbmac structure
 *
 *  Return value:
 *  	   nothing
 ********************************************************************* */

static void sbmac_channel_stop(struct sbmac_softc *s)
{
	/* don't do this if already stopped */

	if (s->sbm_state == sbmac_state_off)
		return;

	/* don't accept any packets, disable all interrupts */

	__raw_writeq(0, s->sbm_rxfilter);
	__raw_writeq(0, s->sbm_imr);

	/* Turn off ticker */

	/* XXX */

	/* turn off receiver and transmitter */

	__raw_writeq(0, s->sbm_macenable);

	/* We're stopped now. */

	s->sbm_state = sbmac_state_off;

	/*
	 * Stop DMA channels (rings should be ok now)
	 */

	sbdma_channel_stop(&(s->sbm_rxdma));
	sbdma_channel_stop(&(s->sbm_txdma));

	/* Empty the receive and transmit rings */

	sbdma_emptyring(&(s->sbm_rxdma));
	sbdma_emptyring(&(s->sbm_txdma));

}

/**********************************************************************
 *  SBMAC_SET_CHANNEL_STATE(state)
 *
 *  Set the channel's state ON or OFF
 *
 *  Input parameters:
 *  	   state - new state
 *
 *  Return value:
 *  	   old state
 ********************************************************************* */
static enum sbmac_state sbmac_set_channel_state(struct sbmac_softc *sc,
						enum sbmac_state state)
{
	enum sbmac_state oldstate = sc->sbm_state;

	/*
	 * If same as previous state, return
	 */

	if (state == oldstate) {
		return oldstate;
	}

	/*
	 * If new state is ON, turn channel on
	 */

	if (state == sbmac_state_on) {
		sbmac_channel_start(sc);
	}
	else {
		sbmac_channel_stop(sc);
	}

	/*
	 * Return previous state
	 */

	return oldstate;
}


/**********************************************************************
 *  SBMAC_PROMISCUOUS_MODE(sc,onoff)
 *
 *  Turn on or off promiscuous mode
 *
 *  Input parameters:
 *  	   sc - softc
 *      onoff - 1 to turn on, 0 to turn off
 *
 *  Return value:
 *  	   nothing
 ********************************************************************* */

static void sbmac_promiscuous_mode(struct sbmac_softc *sc,int onoff)
{
	uint64_t reg;

	if (sc->sbm_state != sbmac_state_on)
		return;

	if (onoff) {
		reg = __raw_readq(sc->sbm_rxfilter);
		reg |= M_MAC_ALLPKT_EN;
		__raw_writeq(reg, sc->sbm_rxfilter);
	}
	else {
		reg = __raw_readq(sc->sbm_rxfilter);
		reg &= ~M_MAC_ALLPKT_EN;
		__raw_writeq(reg, sc->sbm_rxfilter);
	}
}

/**********************************************************************
 *  SBMAC_SETIPHDR_OFFSET(sc,onoff)
 *
 *  Set the iphdr offset as 15 assuming ethernet encapsulation
 *
 *  Input parameters:
 *  	   sc - softc
 *
 *  Return value:
 *  	   nothing
 ********************************************************************* */

static void sbmac_set_iphdr_offset(struct sbmac_softc *sc)
{
	uint64_t reg;

	/* Hard code the off set to 15 for now */
	reg = __raw_readq(sc->sbm_rxfilter);
	reg &= ~M_MAC_IPHDR_OFFSET | V_MAC_IPHDR_OFFSET(15);
	__raw_writeq(reg, sc->sbm_rxfilter);

	/* BCM1250 pass1 didn't have hardware checksum.  Everything
	   later does.  */
	if (soc_type == K_SYS_SOC_TYPE_BCM1250 && periph_rev < 2) {
		sc->rx_hw_checksum = DISABLE;
	} else {
		sc->rx_hw_checksum = ENABLE;
	}