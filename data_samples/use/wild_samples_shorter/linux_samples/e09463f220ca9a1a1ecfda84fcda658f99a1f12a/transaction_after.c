		if (jh->b_transaction == transaction &&
		    jh->b_jlist != BJ_Metadata) {
			jbd_lock_bh_state(bh);
			if (jh->b_transaction == transaction &&
			    jh->b_jlist != BJ_Metadata)
				pr_err("JBD2: assertion failure: h_type=%u "
				       "h_line_no=%u block_no=%llu jlist=%u\n",
				       handle->h_type, handle->h_line_no,
				       (unsigned long long) bh->b_blocknr,
				       jh->b_jlist);
			J_ASSERT_JH(jh, jh->b_transaction != transaction ||
					jh->b_jlist == BJ_Metadata);
			jbd_unlock_bh_state(bh);
		}
		 * of the transaction. This needs to be done
		 * once a transaction -bzzz
		 */
		if (handle->h_buffer_credits <= 0) {
			ret = -ENOSPC;
			goto out_unlock_bh;
		}
		jh->b_modified = 1;
		handle->h_buffer_credits--;
	}

	/*