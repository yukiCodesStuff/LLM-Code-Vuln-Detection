{
	int off = 0;

	buf[off++] = (0x6 << 4);
	buf[off++] = 0x01;
	buf[off++] = 0x40;
	buf[off] = (0x5 << 4);

	spc_parse_naa_6h_vendor_specific(dev, &buf[off]);
	return 0;
}

/**
 * target_xcopy_locate_se_dev_e4_iter - compare XCOPY NAA device identifiers
 *
 * @se_dev: device being considered for match
 * @dev_wwn: XCOPY requested NAA dev_wwn
 * @return: 1 on match, 0 on no-match
 */
static int target_xcopy_locate_se_dev_e4_iter(struct se_device *se_dev,
					      const unsigned char *dev_wwn)
{
	unsigned char tmp_dev_wwn[XCOPY_NAA_IEEE_REGEX_LEN];
	int rc;

	if (!se_dev->dev_attrib.emulate_3pc) {
		pr_debug("XCOPY: emulate_3pc disabled on se_dev %p\n", se_dev);
		return 0;
	}
	switch (xop->op_origin) {
	case XCOL_SOURCE_RECV_OP:
		rc = target_xcopy_locate_se_dev_e4(se_cmd->se_sess,
						xop->dst_tid_wwn,
						&xop->dst_dev,
						&xop->remote_lun_ref);
		break;
	case XCOL_DEST_RECV_OP:
		rc = target_xcopy_locate_se_dev_e4(se_cmd->se_sess,
						xop->src_tid_wwn,
						&xop->src_dev,
						&xop->remote_lun_ref);
		break;
	default:
		pr_err("XCOPY CSCD descriptor IDs not found in CSCD list - "
			"stdi: %hu dtdi: %hu\n", xop->stdi, xop->dtdi);
		rc = -EINVAL;
		break;
	}
{
        return 0;
}

static void xcopy_pt_undepend_remotedev(struct xcopy_op *xop)
{
	if (xop->op_origin == XCOL_SOURCE_RECV_OP)
		pr_debug("putting dst lun_ref for %p\n", xop->dst_dev);
	else
		pr_debug("putting src lun_ref for %p\n", xop->src_dev);

	percpu_ref_put(xop->remote_lun_ref);
}