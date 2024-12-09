		if (jh->b_transaction == transaction &&
		    jh->b_jlist != BJ_Metadata) {
			jbd_lock_bh_state(bh);
			J_ASSERT_JH(jh, jh->b_transaction != transaction ||
					jh->b_jlist == BJ_Metadata);
			jbd_unlock_bh_state(bh);
		}
		 * of the transaction. This needs to be done
		 * once a transaction -bzzz
		 */
		jh->b_modified = 1;
		if (handle->h_buffer_credits <= 0) {
			ret = -ENOSPC;
			goto out_unlock_bh;
		}
		handle->h_buffer_credits--;
	}

	/*