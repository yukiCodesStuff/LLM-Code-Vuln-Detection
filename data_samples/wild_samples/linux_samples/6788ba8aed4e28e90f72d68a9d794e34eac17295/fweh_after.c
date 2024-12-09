	while ((event = brcmf_fweh_dequeue_event(fweh))) {
		brcmf_dbg(EVENT, "event %s (%u) ifidx %u bsscfg %u addr %pM\n",
			  brcmf_fweh_event_name(event->code), event->code,
			  event->emsg.ifidx, event->emsg.bsscfgidx,
			  event->emsg.addr);
		if (event->emsg.bsscfgidx >= BRCMF_MAX_IFS) {
			bphy_err(drvr, "invalid bsscfg index: %u\n", event->emsg.bsscfgidx);
			goto event_free;
		}