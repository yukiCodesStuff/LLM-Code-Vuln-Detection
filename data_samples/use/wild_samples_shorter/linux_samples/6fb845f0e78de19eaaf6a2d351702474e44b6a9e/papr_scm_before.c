{
	unsigned long ret[PLPAR_HCALL_BUFSIZE];
	uint64_t rc, token;

	/*
	 * When the hypervisor cannot map all the requested memory in a single
	 * hcall it returns H_BUSY and we call again with the token until
		rc = plpar_hcall(H_SCM_BIND_MEM, ret, p->drc_index, 0,
				p->blocks, BIND_ANY_ADDR, token);
		token = ret[0];
		cond_resched();
	} while (rc == H_BUSY);

	if (rc) {
		return -ENXIO;
	}

	p->bound_addr = ret[1];

	dev_dbg(&p->pdev->dev, "bound drc %x to %pR\n", p->drc_index, &p->res);

	return 0;