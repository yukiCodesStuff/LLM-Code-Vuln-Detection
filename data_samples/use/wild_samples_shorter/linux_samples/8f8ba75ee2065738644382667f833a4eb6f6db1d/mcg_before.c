			if ((be32_to_cpu(mgm->qp[i]) & MGM_QPN_MASK) == qpn) {
				/* Entry already exists, add to duplicates */
				dqp = kmalloc(sizeof *dqp, GFP_KERNEL);
				if (!dqp)
					goto out_mailbox;
				dqp->qpn = qpn;
				list_add_tail(&dqp->list, &entry->duplicates);
				found = true;
			}