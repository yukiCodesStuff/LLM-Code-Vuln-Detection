{
	unsigned long ret[PLPAR_HCALL_BUFSIZE];
	uint64_t rc, token;

	/*
	 * When the hypervisor cannot map all the requested memory in a single
	 * hcall it returns H_BUSY and we call again with the token until
	 * we get H_SUCCESS. Aborting the retry loop before getting H_SUCCESS
	 * leave the system in an undefined state, so we wait.
	 */
	token = 0;

	do {
		rc = plpar_hcall(H_SCM_BIND_MEM, ret, p->drc_index, 0,
				p->blocks, BIND_ANY_ADDR, token);
		token = ret[0];
		cond_resched();
	} while (rc == H_BUSY);

	if (rc) {
		dev_err(&p->pdev->dev, "bind err: %lld\n", rc);
		return -ENXIO;
	}

	p->bound_addr = ret[1];

	dev_dbg(&p->pdev->dev, "bound drc %x to %pR\n", p->drc_index, &p->res);

	return 0;
}
	do {
		rc = plpar_hcall(H_SCM_BIND_MEM, ret, p->drc_index, 0,
				p->blocks, BIND_ANY_ADDR, token);
		token = ret[0];
		cond_resched();
	} while (rc == H_BUSY);

	if (rc) {
		dev_err(&p->pdev->dev, "bind err: %lld\n", rc);
		return -ENXIO;
	}
	if (rc) {
		dev_err(&p->pdev->dev, "bind err: %lld\n", rc);
		return -ENXIO;
	}

	p->bound_addr = ret[1];

	dev_dbg(&p->pdev->dev, "bound drc %x to %pR\n", p->drc_index, &p->res);

	return 0;
}

static int drc_pmem_unbind(struct papr_scm_priv *p)
{
	unsigned long ret[PLPAR_HCALL_BUFSIZE];
	uint64_t rc, token;

	token = 0;

	/* NB: unbind has the same retry requirements mentioned above */
	do {
		rc = plpar_hcall(H_SCM_UNBIND_MEM, ret, p->drc_index,
				p->bound_addr, p->blocks, token);
		token = ret[0];
		cond_resched();
	} while (rc == H_BUSY);

	if (rc)
		dev_err(&p->pdev->dev, "unbind error: %lld\n", rc);

	return !!rc;
}

static int papr_scm_meta_get(struct papr_scm_priv *p,
			struct nd_cmd_get_config_data_hdr *hdr)
{
	unsigned long data[PLPAR_HCALL_BUFSIZE];
	int64_t ret;

	if (hdr->in_offset >= p->metadata_size || hdr->in_length != 1)
		return -EINVAL;

	ret = plpar_hcall(H_SCM_READ_METADATA, data, p->drc_index,
			hdr->in_offset, 1);

	if (ret == H_PARAMETER) /* bad DRC index */
		return -ENODEV;
	if (ret)
		return -EINVAL; /* other invalid parameter */

	hdr->out_buf[0] = data[0] & 0xff;

	return 0;
}

static int papr_scm_meta_set(struct papr_scm_priv *p,
			struct nd_cmd_set_config_hdr *hdr)
{
	int64_t ret;

	if (hdr->in_offset >= p->metadata_size || hdr->in_length != 1)
		return -EINVAL;

	ret = plpar_hcall_norets(H_SCM_WRITE_METADATA,
			p->drc_index, hdr->in_offset, hdr->in_buf[0], 1);

	if (ret == H_PARAMETER) /* bad DRC index */
		return -ENODEV;
	if (ret)
		return -EINVAL; /* other invalid parameter */

	return 0;
}

int papr_scm_ndctl(struct nvdimm_bus_descriptor *nd_desc, struct nvdimm *nvdimm,
		unsigned int cmd, void *buf, unsigned int buf_len, int *cmd_rc)
{
	struct nd_cmd_get_config_size *get_size_hdr;
	struct papr_scm_priv *p;

	/* Only dimm-specific calls are supported atm */
	if (!nvdimm)
		return -EINVAL;

	p = nvdimm_provider_data(nvdimm);

	switch (cmd) {
	case ND_CMD_GET_CONFIG_SIZE:
		get_size_hdr = buf;

		get_size_hdr->status = 0;
		get_size_hdr->max_xfer = 1;
		get_size_hdr->config_size = p->metadata_size;
		*cmd_rc = 0;
		break;

	case ND_CMD_GET_CONFIG_DATA:
		*cmd_rc = papr_scm_meta_get(p, buf);
		break;

	case ND_CMD_SET_CONFIG_DATA:
		*cmd_rc = papr_scm_meta_set(p, buf);
		break;

	default:
		return -EINVAL;
	}