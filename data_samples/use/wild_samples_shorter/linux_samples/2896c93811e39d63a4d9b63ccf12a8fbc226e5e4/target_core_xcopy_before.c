	return 0;
}

struct xcopy_dev_search_info {
	const unsigned char *dev_wwn;
	struct se_device *found_dev;
};

static int target_xcopy_locate_se_dev_e4_iter(struct se_device *se_dev,
					      void *data)
{
	struct xcopy_dev_search_info *info = data;
	unsigned char tmp_dev_wwn[XCOPY_NAA_IEEE_REGEX_LEN];
	int rc;

	if (!se_dev->dev_attrib.emulate_3pc)
		return 0;

	memset(&tmp_dev_wwn[0], 0, XCOPY_NAA_IEEE_REGEX_LEN);
	target_xcopy_gen_naa_ieee(se_dev, &tmp_dev_wwn[0]);

	rc = memcmp(&tmp_dev_wwn[0], info->dev_wwn, XCOPY_NAA_IEEE_REGEX_LEN);
	if (rc != 0)
		return 0;

	info->found_dev = se_dev;
	pr_debug("XCOPY 0xe4: located se_dev: %p\n", se_dev);

	rc = target_depend_item(&se_dev->dev_group.cg_item);
	if (rc != 0) {
		pr_err("configfs_depend_item attempt failed: %d for se_dev: %p\n",
		       rc, se_dev);
		return rc;
	}

	pr_debug("Called configfs_depend_item for se_dev: %p se_dev->se_dev_group: %p\n",
		 se_dev, &se_dev->dev_group);
	return 1;
}

static int target_xcopy_locate_se_dev_e4(const unsigned char *dev_wwn,
					struct se_device **found_dev)
{
	struct xcopy_dev_search_info info;
	int ret;

	memset(&info, 0, sizeof(info));
	info.dev_wwn = dev_wwn;

	ret = target_for_each_device(target_xcopy_locate_se_dev_e4_iter, &info);
	if (ret == 1) {
		*found_dev = info.found_dev;
		return 0;
	} else {
		pr_debug_ratelimited("Unable to locate 0xe4 descriptor for EXTENDED_COPY\n");
		return -EINVAL;
	}
}

static int target_xcopy_parse_tiddesc_e4(struct se_cmd *se_cmd, struct xcopy_op *xop,
				unsigned char *p, unsigned short cscd_index)

	switch (xop->op_origin) {
	case XCOL_SOURCE_RECV_OP:
		rc = target_xcopy_locate_se_dev_e4(xop->dst_tid_wwn,
						&xop->dst_dev);
		break;
	case XCOL_DEST_RECV_OP:
		rc = target_xcopy_locate_se_dev_e4(xop->src_tid_wwn,
						&xop->src_dev);
		break;
	default:
		pr_err("XCOPY CSCD descriptor IDs not found in CSCD list - "
			"stdi: %hu dtdi: %hu\n", xop->stdi, xop->dtdi);

static void xcopy_pt_undepend_remotedev(struct xcopy_op *xop)
{
	struct se_device *remote_dev;

	if (xop->op_origin == XCOL_SOURCE_RECV_OP)
		remote_dev = xop->dst_dev;
	else
		remote_dev = xop->src_dev;

	pr_debug("Calling configfs_undepend_item for"
		  " remote_dev: %p remote_dev->dev_group: %p\n",
		  remote_dev, &remote_dev->dev_group.cg_item);

	target_undepend_item(&remote_dev->dev_group.cg_item);
}

static void xcopy_pt_release_cmd(struct se_cmd *se_cmd)
{