			if ((be32_to_cpu(mgm->qp[i]) & MGM_QPN_MASK) == qpn) {
				/* Entry already exists, add to duplicates */
				dqp = kmalloc(sizeof *dqp, GFP_KERNEL);
				if (!dqp)
					goto out_mailbox;
				dqp->qpn = qpn;
				list_add_tail(&dqp->list, &entry->duplicates);
				found = true;
			}
		}
		if (!found) {
			/* Need to add the qpn to mgm */
			if (members_count == dev->caps.num_qp_per_mgm) {
				/* entry is full */
				err = -ENOMEM;
				goto out_mailbox;
			}
			mgm->qp[members_count++] = cpu_to_be32(qpn & MGM_QPN_MASK);
			mgm->members_count = cpu_to_be32(members_count | (prot << 30));
			err = mlx4_WRITE_ENTRY(dev, entry->index, mailbox);
			if (err)
				goto out_mailbox;
		}
	}

	/* add the new qpn to list of promisc qps */
	list_add_tail(&pqp->list, &s_steer->promisc_qps[steer]);
	/* now need to add all the promisc qps to default entry */
	memset(mgm, 0, sizeof *mgm);
	members_count = 0;
	list_for_each_entry(dqp, &s_steer->promisc_qps[steer], list)
		mgm->qp[members_count++] = cpu_to_be32(dqp->qpn & MGM_QPN_MASK);
	mgm->members_count = cpu_to_be32(members_count | MLX4_PROT_ETH << 30);

	err = mlx4_WRITE_PROMISC(dev, port, steer, mailbox);
	if (err)
		goto out_list;

	mlx4_free_cmd_mailbox(dev, mailbox);
	mutex_unlock(&priv->mcg_table.mutex);
	return 0;

out_list:
	list_del(&pqp->list);
out_mailbox:
	mlx4_free_cmd_mailbox(dev, mailbox);
out_alloc:
	kfree(pqp);
out_mutex:
	mutex_unlock(&priv->mcg_table.mutex);
	return err;
}

static int remove_promisc_qp(struct mlx4_dev *dev, u8 port,
			     enum mlx4_steer_type steer, u32 qpn)
{
	struct mlx4_priv *priv = mlx4_priv(dev);
	struct mlx4_steer *s_steer;
	struct mlx4_cmd_mailbox *mailbox;
	struct mlx4_mgm *mgm;
	struct mlx4_steer_index *entry;
	struct mlx4_promisc_qp *pqp;
	struct mlx4_promisc_qp *dqp;
	u32 members_count;
	bool found;
	bool back_to_list = false;
	int loc, i;
	int err;

	s_steer = &mlx4_priv(dev)->steer[port - 1];
	mutex_lock(&priv->mcg_table.mutex);

	pqp = get_promisc_qp(dev, 0, steer, qpn);
	if (unlikely(!pqp)) {
		mlx4_warn(dev, "QP %x is not promiscuous QP\n", qpn);
		/* nothing to do */
		err = 0;
		goto out_mutex;
	}

	/*remove from list of promisc qps */
	list_del(&pqp->list);

	/* set the default entry not to include the removed one */
	mailbox = mlx4_alloc_cmd_mailbox(dev);
	if (IS_ERR(mailbox)) {
		err = -ENOMEM;
		back_to_list = true;
		goto out_list;
	}
	mgm = mailbox->buf;
	memset(mgm, 0, sizeof *mgm);
	members_count = 0;
	list_for_each_entry(dqp, &s_steer->promisc_qps[steer], list)
		mgm->qp[members_count++] = cpu_to_be32(dqp->qpn & MGM_QPN_MASK);
	mgm->members_count = cpu_to_be32(members_count | MLX4_PROT_ETH << 30);

	err = mlx4_WRITE_PROMISC(dev, port, steer, mailbox);
	if (err)
		goto out_mailbox;

	/* remove the qp from all the steering entries*/
	list_for_each_entry(entry, &s_steer->steer_entries[steer], list) {
		found = false;
		list_for_each_entry(dqp, &entry->duplicates, list) {
			if (dqp->qpn == qpn) {
				found = true;
				break;
			}
		}
		if (found) {
			/* a duplicate, no need to change the mgm,
			 * only update the duplicates list */
			list_del(&dqp->list);
			kfree(dqp);
		} else {
			err = mlx4_READ_ENTRY(dev, entry->index, mailbox);
				if (err)
					goto out_mailbox;
			members_count = be32_to_cpu(mgm->members_count) & 0xffffff;
			for (loc = -1, i = 0; i < members_count; ++i)
				if ((be32_to_cpu(mgm->qp[i]) & MGM_QPN_MASK) == qpn)
					loc = i;

			mgm->members_count = cpu_to_be32(--members_count |
							 (MLX4_PROT_ETH << 30));
			mgm->qp[loc] = mgm->qp[i - 1];
			mgm->qp[i - 1] = 0;

			err = mlx4_WRITE_ENTRY(dev, entry->index, mailbox);
				if (err)
					goto out_mailbox;
		}

	}

out_mailbox:
	mlx4_free_cmd_mailbox(dev, mailbox);
out_list:
	if (back_to_list)
		list_add_tail(&pqp->list, &s_steer->promisc_qps[steer]);
	else
		kfree(pqp);
out_mutex:
	mutex_unlock(&priv->mcg_table.mutex);
	return err;
}

/*
 * Caller must hold MCG table semaphore.  gid and mgm parameters must
 * be properly aligned for command interface.
 *
 *  Returns 0 unless a firmware command error occurs.
 *
 * If GID is found in MGM or MGM is empty, *index = *hash, *prev = -1
 * and *mgm holds MGM entry.
 *
 * if GID is found in AMGM, *index = index in AMGM, *prev = index of
 * previous entry in hash chain and *mgm holds AMGM entry.
 *
 * If no AMGM exists for given gid, *index = -1, *prev = index of last
 * entry in hash chain and *mgm holds end of hash chain.
 */
static int find_entry(struct mlx4_dev *dev, u8 port,
		      u8 *gid, enum mlx4_protocol prot,
		      struct mlx4_cmd_mailbox *mgm_mailbox,
		      int *prev, int *index)
{
	struct mlx4_cmd_mailbox *mailbox;
	struct mlx4_mgm *mgm = mgm_mailbox->buf;
	u8 *mgid;
	int err;
	u16 hash;
	u8 op_mod = (prot == MLX4_PROT_ETH) ?
		!!(dev->caps.flags & MLX4_DEV_CAP_FLAG_VEP_MC_STEER) : 0;

	mailbox = mlx4_alloc_cmd_mailbox(dev);
	if (IS_ERR(mailbox))
		return -ENOMEM;
	mgid = mailbox->buf;

	memcpy(mgid, gid, 16);

	err = mlx4_GID_HASH(dev, mailbox, &hash, op_mod);
	mlx4_free_cmd_mailbox(dev, mailbox);
	if (err)
		return err;

	if (0)
		mlx4_dbg(dev, "Hash for %pI6 is %04x\n", gid, hash);

	*index = hash;
	*prev  = -1;

	do {
		err = mlx4_READ_ENTRY(dev, *index, mgm_mailbox);
		if (err)
			return err;

		if (!(be32_to_cpu(mgm->members_count) & 0xffffff)) {
			if (*index != hash) {
				mlx4_err(dev, "Found zero MGID in AMGM.\n");
				err = -EINVAL;
			}