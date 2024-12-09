{
	struct sock *s = unix_get_socket(fp);

	spin_lock(&unix_gc_lock);

	if (s) {
		struct unix_sock *u = unix_sk(s);

		if (atomic_long_inc_return(&u->inflight) == 1) {
			BUG_ON(!list_empty(&u->link));
			list_add_tail(&u->link, &gc_inflight_list);
		} else {
			BUG_ON(list_empty(&u->link));
		}
		unix_tot_inflight++;
	}
	fp->f_cred->user->unix_inflight++;
	spin_unlock(&unix_gc_lock);
}

void unix_notinflight(struct file *fp)
{
	struct sock *s = unix_get_socket(fp);

	spin_lock(&unix_gc_lock);

	if (s) {
		struct unix_sock *u = unix_sk(s);

		BUG_ON(list_empty(&u->link));

		if (atomic_long_dec_and_test(&u->inflight))
			list_del_init(&u->link);
		unix_tot_inflight--;
	}
	fp->f_cred->user->unix_inflight--;
	spin_unlock(&unix_gc_lock);
}

static void scan_inflight(struct sock *x, void (*func)(struct unix_sock *),
			  struct sk_buff_head *hitlist)