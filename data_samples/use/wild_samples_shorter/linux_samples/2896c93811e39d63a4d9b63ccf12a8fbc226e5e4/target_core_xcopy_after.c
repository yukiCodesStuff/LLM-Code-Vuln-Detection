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

	memset(&tmp_dev_wwn[0], 0, XCOPY_NAA_IEEE_REGEX_LEN);
	target_xcopy_gen_naa_ieee(se_dev, &tmp_dev_wwn[0]);

	rc = memcmp(&tmp_dev_wwn[0], dev_wwn, XCOPY_NAA_IEEE_REGEX_LEN);
	if (rc != 0) {
		pr_debug("XCOPY: skip non-matching: %*ph\n",
			 XCOPY_NAA_IEEE_REGEX_LEN, tmp_dev_wwn);
		return 0;
	}
	pr_debug("XCOPY 0xe4: located se_dev: %p\n", se_dev);

	return 1;
}

static int target_xcopy_locate_se_dev_e4(struct se_session *sess,
					const unsigned char *dev_wwn,
					struct se_device **_found_dev,
					struct percpu_ref **_found_lun_ref)
{
	struct se_dev_entry *deve;
	struct se_node_acl *nacl;
	struct se_lun *this_lun = NULL;
	struct se_device *found_dev = NULL;

	/* cmd with NULL sess indicates no associated $FABRIC_MOD */
	if (!sess)
		goto err_out;

	pr_debug("XCOPY 0xe4: searching for: %*ph\n",
		 XCOPY_NAA_IEEE_REGEX_LEN, dev_wwn);

	nacl = sess->se_node_acl;
	rcu_read_lock();
	hlist_for_each_entry_rcu(deve, &nacl->lun_entry_hlist, link) {
		struct se_device *this_dev;
		int rc;

		this_lun = rcu_dereference(deve->se_lun);
		this_dev = rcu_dereference_raw(this_lun->lun_se_dev);

		rc = target_xcopy_locate_se_dev_e4_iter(this_dev, dev_wwn);
		if (rc) {
			if (percpu_ref_tryget_live(&this_lun->lun_ref))
				found_dev = this_dev;
			break;
		}
	}
	rcu_read_unlock();
	if (found_dev == NULL)
		goto err_out;

	pr_debug("lun_ref held for se_dev: %p se_dev->se_dev_group: %p\n",
		 found_dev, &found_dev->dev_group);
	*_found_dev = found_dev;
	*_found_lun_ref = &this_lun->lun_ref;
	return 0;
err_out:
	pr_debug_ratelimited("Unable to locate 0xe4 descriptor for EXTENDED_COPY\n");
	return -EINVAL;
}

static int target_xcopy_parse_tiddesc_e4(struct se_cmd *se_cmd, struct xcopy_op *xop,
				unsigned char *p, unsigned short cscd_index)

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

static void xcopy_pt_undepend_remotedev(struct xcopy_op *xop)
{
	if (xop->op_origin == XCOL_SOURCE_RECV_OP)
		pr_debug("putting dst lun_ref for %p\n", xop->dst_dev);
	else
		pr_debug("putting src lun_ref for %p\n", xop->src_dev);

	percpu_ref_put(xop->remote_lun_ref);
}

static void xcopy_pt_release_cmd(struct se_cmd *se_cmd)
{