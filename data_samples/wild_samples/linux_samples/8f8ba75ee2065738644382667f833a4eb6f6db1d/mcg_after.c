			if ((be32_to_cpu(mgm->qp[i]) & MGM_QPN_MASK) == qpn) {
				/* Entry already exists, add to duplicates */
				dqp = kmalloc(sizeof *dqp, GFP_KERNEL);
				if (!dqp) {
					err = -ENOMEM;
					goto out_mailbox;
				}