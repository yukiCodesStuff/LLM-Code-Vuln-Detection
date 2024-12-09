{
	struct rb_node *_p = v;
	struct key *key = rb_entry(_p, struct key, serial_node);
	struct timespec now;
	unsigned long timo;
	key_ref_t key_ref, skey_ref;
	char xbuf[16];
	int rc;

	struct keyring_search_context ctx = {
		.index_key.type		= key->type,
		.index_key.description	= key->description,
		.cred			= current_cred(),
		.match_data.cmp		= lookup_user_key_possessed,
		.match_data.raw_data	= key,
		.match_data.lookup_type	= KEYRING_SEARCH_LOOKUP_DIRECT,
		.flags			= KEYRING_SEARCH_NO_STATE_CHECK,
	};

	key_ref = make_key_ref(key, 0);

	/* determine if the key is possessed by this process (a test we can
	 * skip if the key does not indicate the possessor can view it
	 */
	if (key->perm & KEY_POS_VIEW) {
		skey_ref = search_my_process_keyrings(&ctx);
		if (!IS_ERR(skey_ref)) {
			key_ref_put(skey_ref);
			key_ref = make_key_ref(key, 1);
		}
	}

	/* check whether the current task is allowed to view the key (assuming
	 * non-possession)
	 * - the caller holds a spinlock, and thus the RCU read lock, making our
	 *   access to __current_cred() safe
	 */
	rc = key_task_permission(key_ref, ctx.cred, KEY_NEED_VIEW);
	if (rc < 0)
		return 0;

	now = current_kernel_time();

	rcu_read_lock();

	/* come up with a suitable timeout value */
	if (key->expiry == 0) {
		memcpy(xbuf, "perm", 5);
	} else if (now.tv_sec >= key->expiry) {
		memcpy(xbuf, "expd", 5);
	} else {
		timo = key->expiry - now.tv_sec;

		if (timo < 60)
			sprintf(xbuf, "%lus", timo);
		else if (timo < 60*60)
			sprintf(xbuf, "%lum", timo / 60);
		else if (timo < 60*60*24)
			sprintf(xbuf, "%luh", timo / (60*60));
		else if (timo < 60*60*24*7)
			sprintf(xbuf, "%lud", timo / (60*60*24));
		else
			sprintf(xbuf, "%luw", timo / (60*60*24*7));
	}

#define showflag(KEY, LETTER, FLAG) \
	(test_bit(FLAG,	&(KEY)->flags) ? LETTER : '-')

	seq_printf(m, "%08x %c%c%c%c%c%c%c %5d %4s %08x %5d %5d %-9.9s ",
		   key->serial,
		   showflag(key, 'I', KEY_FLAG_INSTANTIATED),
		   showflag(key, 'R', KEY_FLAG_REVOKED),
		   showflag(key, 'D', KEY_FLAG_DEAD),
		   showflag(key, 'Q', KEY_FLAG_IN_QUOTA),
		   showflag(key, 'U', KEY_FLAG_USER_CONSTRUCT),
		   showflag(key, 'N', KEY_FLAG_NEGATIVE),
		   showflag(key, 'i', KEY_FLAG_INVALIDATED),
		   atomic_read(&key->usage),
		   xbuf,
		   key->perm,
		   from_kuid_munged(seq_user_ns(m), key->uid),
		   from_kgid_munged(seq_user_ns(m), key->gid),
		   key->type->name);

#undef showflag

	if (key->type->describe)
		key->type->describe(key, m);
	seq_putc(m, '\n');

	rcu_read_unlock();
	return 0;
}

static struct rb_node *__key_user_next(struct user_namespace *user_ns, struct rb_node *n)
{
	while (n) {
		struct key_user *user = rb_entry(n, struct key_user, node);
		if (kuid_has_mapping(user_ns, user->uid))
			break;
		n = rb_next(n);
	}
	return n;
}

static struct rb_node *key_user_next(struct user_namespace *user_ns, struct rb_node *n)
{
	return __key_user_next(user_ns, rb_next(n));
}

static struct rb_node *key_user_first(struct user_namespace *user_ns, struct rb_root *r)
{
	struct rb_node *n = rb_first(r);
	return __key_user_next(user_ns, n);
}

/*
 * Implement "/proc/key-users" to provides a list of the key users and their
 * quotas.
 */
static int proc_key_users_open(struct inode *inode, struct file *file)
{
	return seq_open(file, &proc_key_users_ops);
}

static void *proc_key_users_start(struct seq_file *p, loff_t *_pos)
	__acquires(key_user_lock)
{
	struct rb_node *_p;
	loff_t pos = *_pos;

	spin_lock(&key_user_lock);

	_p = key_user_first(seq_user_ns(p), &key_user_tree);
	while (pos > 0 && _p) {
		pos--;
		_p = key_user_next(seq_user_ns(p), _p);
	}

	return _p;
}

static void *proc_key_users_next(struct seq_file *p, void *v, loff_t *_pos)
{
	(*_pos)++;
	return key_user_next(seq_user_ns(p), (struct rb_node *)v);
}

static void proc_key_users_stop(struct seq_file *p, void *v)
	__releases(key_user_lock)
{
	spin_unlock(&key_user_lock);
}

static int proc_key_users_show(struct seq_file *m, void *v)
{
	struct rb_node *_p = v;
	struct key_user *user = rb_entry(_p, struct key_user, node);
	unsigned maxkeys = uid_eq(user->uid, GLOBAL_ROOT_UID) ?
		key_quota_root_maxkeys : key_quota_maxkeys;
	unsigned maxbytes = uid_eq(user->uid, GLOBAL_ROOT_UID) ?
		key_quota_root_maxbytes : key_quota_maxbytes;

	seq_printf(m, "%5u: %5d %d/%d %d/%d %d/%d\n",
		   from_kuid_munged(seq_user_ns(m), user->uid),
		   atomic_read(&user->usage),
		   atomic_read(&user->nkeys),
		   atomic_read(&user->nikeys),
		   user->qnkeys,
		   maxkeys,
		   user->qnbytes,
		   maxbytes);

	return 0;
}