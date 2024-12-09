			  brcmf_fweh_event_name(event->code), event->code,
			  event->emsg.ifidx, event->emsg.bsscfgidx,
			  event->emsg.addr);
		if (event->emsg.bsscfgidx >= BRCMF_MAX_IFS) {
			bphy_err(drvr, "invalid bsscfg index: %u\n", event->emsg.bsscfgidx);
			goto event_free;
		}

		/* convert event message */
		emsg_be = &event->emsg;
		emsg.version = be16_to_cpu(emsg_be->version);