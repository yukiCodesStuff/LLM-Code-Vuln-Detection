{
	struct ctl_table_header *head = grab_header(file_inode(file));
	struct ctl_table_header *h = NULL;
	struct ctl_table *entry;
	struct ctl_dir *ctl_dir;
	unsigned long pos;

	if (IS_ERR(head))
		return PTR_ERR(head);

	ctl_dir = container_of(head, struct ctl_dir, header);

	if (!dir_emit_dots(file, ctx))
		return 0;

	pos = 2;

	for (first_entry(ctl_dir, &h, &entry); h; next_entry(&h, &entry)) {
		if (!scan(h, entry, &pos, file, ctx)) {
			sysctl_head_finish(h);
			break;
		}
	}
		if (!scan(h, entry, &pos, file, ctx)) {
			sysctl_head_finish(h);
			break;
		}
	}
	sysctl_head_finish(head);
	return 0;
}

static int proc_sys_permission(struct inode *inode, int mask)
{
	/*
	 * sysctl entries that are not writeable,
	 * are _NOT_ writeable, capabilities or not.
	 */
	struct ctl_table_header *head;
	struct ctl_table *table;
	int error;

	/* Executable files are not allowed under /proc/sys/ */
	if ((mask & MAY_EXEC) && S_ISREG(inode->i_mode))
		return -EACCES;

	head = grab_header(inode);
	if (IS_ERR(head))
		return PTR_ERR(head);

	table = PROC_I(inode)->sysctl_entry;
	if (!table) /* global root - r-xr-xr-x */
		error = mask & MAY_WRITE ? -EACCES : 0;
	else /* Use the permissions on the sysctl table entry */
		error = sysctl_perm(head, table, mask & ~MAY_NOT_BLOCK);

	sysctl_head_finish(head);
	return error;
}

static int proc_sys_setattr(struct dentry *dentry, struct iattr *attr)
{
	struct inode *inode = d_inode(dentry);
	int error;

	if (attr->ia_valid & (ATTR_MODE | ATTR_UID | ATTR_GID))
		return -EPERM;

	error = setattr_prepare(dentry, attr);
	if (error)
		return error;

	setattr_copy(inode, attr);
	mark_inode_dirty(inode);
	return 0;
}

static int proc_sys_getattr(struct vfsmount *mnt, struct dentry *dentry, struct kstat *stat)
{
	struct inode *inode = d_inode(dentry);
	struct ctl_table_header *head = grab_header(inode);
	struct ctl_table *table = PROC_I(inode)->sysctl_entry;

	if (IS_ERR(head))
		return PTR_ERR(head);

	generic_fillattr(inode, stat);
	if (table)
		stat->mode = (stat->mode & S_IFMT) | table->mode;

	sysctl_head_finish(head);
	return 0;
}

static const struct file_operations proc_sys_file_operations = {
	.open		= proc_sys_open,
	.poll		= proc_sys_poll,
	.read		= proc_sys_read,
	.write		= proc_sys_write,
	.llseek		= default_llseek,
};

static const struct file_operations proc_sys_dir_file_operations = {
	.read		= generic_read_dir,
	.iterate_shared	= proc_sys_readdir,
	.llseek		= generic_file_llseek,
};

static const struct inode_operations proc_sys_inode_operations = {
	.permission	= proc_sys_permission,
	.setattr	= proc_sys_setattr,
	.getattr	= proc_sys_getattr,
};

static const struct inode_operations proc_sys_dir_operations = {
	.lookup		= proc_sys_lookup,
	.permission	= proc_sys_permission,
	.setattr	= proc_sys_setattr,
	.getattr	= proc_sys_getattr,
};

static int proc_sys_revalidate(struct dentry *dentry, unsigned int flags)
{
	if (flags & LOOKUP_RCU)
		return -ECHILD;
	return !PROC_I(d_inode(dentry))->sysctl->unregistering;
}

static int proc_sys_delete(const struct dentry *dentry)
{
	return !!PROC_I(d_inode(dentry))->sysctl->unregistering;
}

static int sysctl_is_seen(struct ctl_table_header *p)
{
	struct ctl_table_set *set = p->set;
	int res;
	spin_lock(&sysctl_lock);
	if (p->unregistering)
		res = 0;
	else if (!set->is_seen)
		res = 1;
	else
		res = set->is_seen(set);
	spin_unlock(&sysctl_lock);
	return res;
}

static int proc_sys_compare(const struct dentry *dentry,
		unsigned int len, const char *str, const struct qstr *name)
{
	struct ctl_table_header *head;
	struct inode *inode;

	/* Although proc doesn't have negative dentries, rcu-walk means
	 * that inode here can be NULL */
	/* AV: can it, indeed? */
	inode = d_inode_rcu(dentry);
	if (!inode)
		return 1;
	if (name->len != len)
		return 1;
	if (memcmp(name->name, str, len))
		return 1;
	head = rcu_dereference(PROC_I(inode)->sysctl);
	return !head || !sysctl_is_seen(head);
}

static const struct dentry_operations proc_sys_dentry_operations = {
	.d_revalidate	= proc_sys_revalidate,
	.d_delete	= proc_sys_delete,
	.d_compare	= proc_sys_compare,
};

static struct ctl_dir *find_subdir(struct ctl_dir *dir,
				   const char *name, int namelen)
{
	struct ctl_table_header *head;
	struct ctl_table *entry;

	entry = find_entry(&head, dir, name, namelen);
	if (!entry)
		return ERR_PTR(-ENOENT);
	if (!S_ISDIR(entry->mode))
		return ERR_PTR(-ENOTDIR);
	return container_of(head, struct ctl_dir, header);
}

static struct ctl_dir *new_dir(struct ctl_table_set *set,
			       const char *name, int namelen)
{
	struct ctl_table *table;
	struct ctl_dir *new;
	struct ctl_node *node;
	char *new_name;

	new = kzalloc(sizeof(*new) + sizeof(struct ctl_node) +
		      sizeof(struct ctl_table)*2 +  namelen + 1,
		      GFP_KERNEL);
	if (!new)
		return NULL;

	node = (struct ctl_node *)(new + 1);
	table = (struct ctl_table *)(node + 1);
	new_name = (char *)(table + 2);
	memcpy(new_name, name, namelen);
	new_name[namelen] = '\0';
	table[0].procname = new_name;
	table[0].mode = S_IFDIR|S_IRUGO|S_IXUGO;
	init_header(&new->header, set->dir.header.root, set, node, table);

	return new;
}

/**
 * get_subdir - find or create a subdir with the specified name.
 * @dir:  Directory to create the subdirectory in
 * @name: The name of the subdirectory to find or create
 * @namelen: The length of name
 *
 * Takes a directory with an elevated reference count so we know that
 * if we drop the lock the directory will not go away.  Upon success
 * the reference is moved from @dir to the returned subdirectory.
 * Upon error an error code is returned and the reference on @dir is
 * simply dropped.
 */
static struct ctl_dir *get_subdir(struct ctl_dir *dir,
				  const char *name, int namelen)
{
	struct ctl_table_set *set = dir->header.set;
	struct ctl_dir *subdir, *new = NULL;
	int err;

	spin_lock(&sysctl_lock);
	subdir = find_subdir(dir, name, namelen);
	if (!IS_ERR(subdir))
		goto found;
	if (PTR_ERR(subdir) != -ENOENT)
		goto failed;

	spin_unlock(&sysctl_lock);
	new = new_dir(set, name, namelen);
	spin_lock(&sysctl_lock);
	subdir = ERR_PTR(-ENOMEM);
	if (!new)
		goto failed;

	/* Was the subdir added while we dropped the lock? */
	subdir = find_subdir(dir, name, namelen);
	if (!IS_ERR(subdir))
		goto found;
	if (PTR_ERR(subdir) != -ENOENT)
		goto failed;

	/* Nope.  Use the our freshly made directory entry. */
	err = insert_header(dir, &new->header);
	subdir = ERR_PTR(err);
	if (err)
		goto failed;
	subdir = new;
found:
	subdir->header.nreg++;
failed:
	if (IS_ERR(subdir)) {
		pr_err("sysctl could not get directory: ");
		sysctl_print_dir(dir);
		pr_cont("/%*.*s %ld\n",
			namelen, namelen, name, PTR_ERR(subdir));
	}
	drop_sysctl_table(&dir->header);
	if (new)
		drop_sysctl_table(&new->header);
	spin_unlock(&sysctl_lock);
	return subdir;
}

static struct ctl_dir *xlate_dir(struct ctl_table_set *set, struct ctl_dir *dir)
{
	struct ctl_dir *parent;
	const char *procname;
	if (!dir->header.parent)
		return &set->dir;
	parent = xlate_dir(set, dir->header.parent);
	if (IS_ERR(parent))
		return parent;
	procname = dir->header.ctl_table[0].procname;
	return find_subdir(parent, procname, strlen(procname));
}

static int sysctl_follow_link(struct ctl_table_header **phead,
	struct ctl_table **pentry)
{
	struct ctl_table_header *head;
	struct ctl_table_root *root;
	struct ctl_table_set *set;
	struct ctl_table *entry;
	struct ctl_dir *dir;
	int ret;

	ret = 0;
	spin_lock(&sysctl_lock);
	root = (*pentry)->data;
	set = lookup_header_set(root);
	dir = xlate_dir(set, (*phead)->parent);
	if (IS_ERR(dir))
		ret = PTR_ERR(dir);
	else {
		const char *procname = (*pentry)->procname;
		head = NULL;
		entry = find_entry(&head, dir, procname, strlen(procname));
		ret = -ENOENT;
		if (entry && use_table(head)) {
			unuse_table(*phead);
			*phead = head;
			*pentry = entry;
			ret = 0;
		}
	}

	spin_unlock(&sysctl_lock);
	return ret;
}

static int sysctl_err(const char *path, struct ctl_table *table, char *fmt, ...)
{
	struct va_format vaf;
	va_list args;

	va_start(args, fmt);
	vaf.fmt = fmt;
	vaf.va = &args;

	pr_err("sysctl table check failed: %s/%s %pV\n",
	       path, table->procname, &vaf);

	va_end(args);
	return -EINVAL;
}

static int sysctl_check_table(const char *path, struct ctl_table *table)
{
	int err = 0;
	for (; table->procname; table++) {
		if (table->child)
			err = sysctl_err(path, table, "Not a file");

		if ((table->proc_handler == proc_dostring) ||
		    (table->proc_handler == proc_dointvec) ||
		    (table->proc_handler == proc_dointvec_minmax) ||
		    (table->proc_handler == proc_dointvec_jiffies) ||
		    (table->proc_handler == proc_dointvec_userhz_jiffies) ||
		    (table->proc_handler == proc_dointvec_ms_jiffies) ||
		    (table->proc_handler == proc_doulongvec_minmax) ||
		    (table->proc_handler == proc_doulongvec_ms_jiffies_minmax)) {
			if (!table->data)
				err = sysctl_err(path, table, "No data");
			if (!table->maxlen)
				err = sysctl_err(path, table, "No maxlen");
		}
		if (!table->proc_handler)
			err = sysctl_err(path, table, "No proc_handler");

		if ((table->mode & (S_IRUGO|S_IWUGO)) != table->mode)
			err = sysctl_err(path, table, "bogus .mode 0%o",
				table->mode);
	}
	return err;
}

static struct ctl_table_header *new_links(struct ctl_dir *dir, struct ctl_table *table,
	struct ctl_table_root *link_root)
{
	struct ctl_table *link_table, *entry, *link;
	struct ctl_table_header *links;
	struct ctl_node *node;
	char *link_name;
	int nr_entries, name_bytes;

	name_bytes = 0;
	nr_entries = 0;
	for (entry = table; entry->procname; entry++) {
		nr_entries++;
		name_bytes += strlen(entry->procname) + 1;
	}

	links = kzalloc(sizeof(struct ctl_table_header) +
			sizeof(struct ctl_node)*nr_entries +
			sizeof(struct ctl_table)*(nr_entries + 1) +
			name_bytes,
			GFP_KERNEL);

	if (!links)
		return NULL;

	node = (struct ctl_node *)(links + 1);
	link_table = (struct ctl_table *)(node + nr_entries);
	link_name = (char *)&link_table[nr_entries + 1];

	for (link = link_table, entry = table; entry->procname; link++, entry++) {
		int len = strlen(entry->procname) + 1;
		memcpy(link_name, entry->procname, len);
		link->procname = link_name;
		link->mode = S_IFLNK|S_IRWXUGO;
		link->data = link_root;
		link_name += len;
	}
	init_header(links, dir->header.root, dir->header.set, node, link_table);
	links->nreg = nr_entries;

	return links;
}

static bool get_links(struct ctl_dir *dir,
	struct ctl_table *table, struct ctl_table_root *link_root)
{
	struct ctl_table_header *head;
	struct ctl_table *entry, *link;

	/* Are there links available for every entry in table? */
	for (entry = table; entry->procname; entry++) {
		const char *procname = entry->procname;
		link = find_entry(&head, dir, procname, strlen(procname));
		if (!link)
			return false;
		if (S_ISDIR(link->mode) && S_ISDIR(entry->mode))
			continue;
		if (S_ISLNK(link->mode) && (link->data == link_root))
			continue;
		return false;
	}

	/* The checks passed.  Increase the registration count on the links */
	for (entry = table; entry->procname; entry++) {
		const char *procname = entry->procname;
		link = find_entry(&head, dir, procname, strlen(procname));
		head->nreg++;
	}
	return true;
}

static int insert_links(struct ctl_table_header *head)
{
	struct ctl_table_set *root_set = &sysctl_table_root.default_set;
	struct ctl_dir *core_parent = NULL;
	struct ctl_table_header *links;
	int err;

	if (head->set == root_set)
		return 0;

	core_parent = xlate_dir(root_set, head->parent);
	if (IS_ERR(core_parent))
		return 0;

	if (get_links(core_parent, head->ctl_table, head->root))
		return 0;

	core_parent->header.nreg++;
	spin_unlock(&sysctl_lock);

	links = new_links(core_parent, head->ctl_table, head->root);

	spin_lock(&sysctl_lock);
	err = -ENOMEM;
	if (!links)
		goto out;

	err = 0;
	if (get_links(core_parent, head->ctl_table, head->root)) {
		kfree(links);
		goto out;
	}

	err = insert_header(core_parent, links);
	if (err)
		kfree(links);
out:
	drop_sysctl_table(&core_parent->header);
	return err;
}

/**
 * __register_sysctl_table - register a leaf sysctl table
 * @set: Sysctl tree to register on
 * @path: The path to the directory the sysctl table is in.
 * @table: the top-level table structure
 *
 * Register a sysctl table hierarchy. @table should be a filled in ctl_table
 * array. A completely 0 filled entry terminates the table.
 *
 * The members of the &struct ctl_table structure are used as follows:
 *
 * procname - the name of the sysctl file under /proc/sys. Set to %NULL to not
 *            enter a sysctl file
 *
 * data - a pointer to data for use by proc_handler
 *
 * maxlen - the maximum size in bytes of the data
 *
 * mode - the file permissions for the /proc/sys file
 *
 * child - must be %NULL.
 *
 * proc_handler - the text handler routine (described below)
 *
 * extra1, extra2 - extra pointers usable by the proc handler routines
 *
 * Leaf nodes in the sysctl tree will be represented by a single file
 * under /proc; non-leaf nodes will be represented by directories.
 *
 * There must be a proc_handler routine for any terminal nodes.
 * Several default handlers are available to cover common cases -
 *
 * proc_dostring(), proc_dointvec(), proc_dointvec_jiffies(),
 * proc_dointvec_userhz_jiffies(), proc_dointvec_minmax(),
 * proc_doulongvec_ms_jiffies_minmax(), proc_doulongvec_minmax()
 *
 * It is the handler's job to read the input buffer from user memory
 * and process it. The handler should return 0 on success.
 *
 * This routine returns %NULL on a failure to register, and a pointer
 * to the table header on success.
 */
struct ctl_table_header *__register_sysctl_table(
	struct ctl_table_set *set,
	const char *path, struct ctl_table *table)
{
	struct ctl_table_root *root = set->dir.header.root;
	struct ctl_table_header *header;
	const char *name, *nextname;
	struct ctl_dir *dir;
	struct ctl_table *entry;
	struct ctl_node *node;
	int nr_entries = 0;

	for (entry = table; entry->procname; entry++)
		nr_entries++;

	header = kzalloc(sizeof(struct ctl_table_header) +
			 sizeof(struct ctl_node)*nr_entries, GFP_KERNEL);
	if (!header)
		return NULL;

	node = (struct ctl_node *)(header + 1);
	init_header(header, root, set, node, table);
	if (sysctl_check_table(path, table))
		goto fail;

	spin_lock(&sysctl_lock);
	dir = &set->dir;
	/* Reference moved down the diretory tree get_subdir */
	dir->header.nreg++;
	spin_unlock(&sysctl_lock);

	/* Find the directory for the ctl_table */
	for (name = path; name; name = nextname) {
		int namelen;
		nextname = strchr(name, '/');
		if (nextname) {
			namelen = nextname - name;
			nextname++;
		} else {
			namelen = strlen(name);
		}
		if (namelen == 0)
			continue;

		dir = get_subdir(dir, name, namelen);
		if (IS_ERR(dir))
			goto fail;
	}

	spin_lock(&sysctl_lock);
	if (insert_header(dir, header))
		goto fail_put_dir_locked;

	drop_sysctl_table(&dir->header);
	spin_unlock(&sysctl_lock);

	return header;

fail_put_dir_locked:
	drop_sysctl_table(&dir->header);
	spin_unlock(&sysctl_lock);
fail:
	kfree(header);
	dump_stack();
	return NULL;
}

/**
 * register_sysctl - register a sysctl table
 * @path: The path to the directory the sysctl table is in.
 * @table: the table structure
 *
 * Register a sysctl table. @table should be a filled in ctl_table
 * array. A completely 0 filled entry terminates the table.
 *
 * See __register_sysctl_table for more details.
 */
struct ctl_table_header *register_sysctl(const char *path, struct ctl_table *table)
{
	return __register_sysctl_table(&sysctl_table_root.default_set,
					path, table);
}
EXPORT_SYMBOL(register_sysctl);

static char *append_path(const char *path, char *pos, const char *name)
{
	int namelen;
	namelen = strlen(name);
	if (((pos - path) + namelen + 2) >= PATH_MAX)
		return NULL;
	memcpy(pos, name, namelen);
	pos[namelen] = '/';
	pos[namelen + 1] = '\0';
	pos += namelen + 1;
	return pos;
}

static int count_subheaders(struct ctl_table *table)
{
	int has_files = 0;
	int nr_subheaders = 0;
	struct ctl_table *entry;

	/* special case: no directory and empty directory */
	if (!table || !table->procname)
		return 1;

	for (entry = table; entry->procname; entry++) {
		if (entry->child)
			nr_subheaders += count_subheaders(entry->child);
		else
			has_files = 1;
	}
	return nr_subheaders + has_files;
}

static int register_leaf_sysctl_tables(const char *path, char *pos,
	struct ctl_table_header ***subheader, struct ctl_table_set *set,
	struct ctl_table *table)
{
	struct ctl_table *ctl_table_arg = NULL;
	struct ctl_table *entry, *files;
	int nr_files = 0;
	int nr_dirs = 0;
	int err = -ENOMEM;

	for (entry = table; entry->procname; entry++) {
		if (entry->child)
			nr_dirs++;
		else
			nr_files++;
	}

	files = table;
	/* If there are mixed files and directories we need a new table */
	if (nr_dirs && nr_files) {
		struct ctl_table *new;
		files = kzalloc(sizeof(struct ctl_table) * (nr_files + 1),
				GFP_KERNEL);
		if (!files)
			goto out;

		ctl_table_arg = files;
		for (new = files, entry = table; entry->procname; entry++) {
			if (entry->child)
				continue;
			*new = *entry;
			new++;
		}
	}

	/* Register everything except a directory full of subdirectories */
	if (nr_files || !nr_dirs) {
		struct ctl_table_header *header;
		header = __register_sysctl_table(set, path, files);
		if (!header) {
			kfree(ctl_table_arg);
			goto out;
		}

		/* Remember if we need to free the file table */
		header->ctl_table_arg = ctl_table_arg;
		**subheader = header;
		(*subheader)++;
	}

	/* Recurse into the subdirectories. */
	for (entry = table; entry->procname; entry++) {
		char *child_pos;

		if (!entry->child)
			continue;

		err = -ENAMETOOLONG;
		child_pos = append_path(path, pos, entry->procname);
		if (!child_pos)
			goto out;

		err = register_leaf_sysctl_tables(path, child_pos, subheader,
						  set, entry->child);
		pos[0] = '\0';
		if (err)
			goto out;
	}
	err = 0;
out:
	/* On failure our caller will unregister all registered subheaders */
	return err;
}

/**
 * __register_sysctl_paths - register a sysctl table hierarchy
 * @set: Sysctl tree to register on
 * @path: The path to the directory the sysctl table is in.
 * @table: the top-level table structure
 *
 * Register a sysctl table hierarchy. @table should be a filled in ctl_table
 * array. A completely 0 filled entry terminates the table.
 *
 * See __register_sysctl_table for more details.
 */
struct ctl_table_header *__register_sysctl_paths(
	struct ctl_table_set *set,
	const struct ctl_path *path, struct ctl_table *table)
{
	struct ctl_table *ctl_table_arg = table;
	int nr_subheaders = count_subheaders(table);
	struct ctl_table_header *header = NULL, **subheaders, **subheader;
	const struct ctl_path *component;
	char *new_path, *pos;

	pos = new_path = kmalloc(PATH_MAX, GFP_KERNEL);
	if (!new_path)
		return NULL;

	pos[0] = '\0';
	for (component = path; component->procname; component++) {
		pos = append_path(new_path, pos, component->procname);
		if (!pos)
			goto out;
	}
	while (table->procname && table->child && !table[1].procname) {
		pos = append_path(new_path, pos, table->procname);
		if (!pos)
			goto out;
		table = table->child;
	}
	if (nr_subheaders == 1) {
		header = __register_sysctl_table(set, new_path, table);
		if (header)
			header->ctl_table_arg = ctl_table_arg;
	} else {
		header = kzalloc(sizeof(*header) +
				 sizeof(*subheaders)*nr_subheaders, GFP_KERNEL);
		if (!header)
			goto out;

		subheaders = (struct ctl_table_header **) (header + 1);
		subheader = subheaders;
		header->ctl_table_arg = ctl_table_arg;

		if (register_leaf_sysctl_tables(new_path, pos, &subheader,
						set, table))
			goto err_register_leaves;
	}

out:
	kfree(new_path);
	return header;

err_register_leaves:
	while (subheader > subheaders) {
		struct ctl_table_header *subh = *(--subheader);
		struct ctl_table *table = subh->ctl_table_arg;
		unregister_sysctl_table(subh);
		kfree(table);
	}
	kfree(header);
	header = NULL;
	goto out;
}

/**
 * register_sysctl_table_path - register a sysctl table hierarchy
 * @path: The path to the directory the sysctl table is in.
 * @table: the top-level table structure
 *
 * Register a sysctl table hierarchy. @table should be a filled in ctl_table
 * array. A completely 0 filled entry terminates the table.
 *
 * See __register_sysctl_paths for more details.
 */
struct ctl_table_header *register_sysctl_paths(const struct ctl_path *path,
						struct ctl_table *table)
{
	return __register_sysctl_paths(&sysctl_table_root.default_set,
					path, table);
}
EXPORT_SYMBOL(register_sysctl_paths);

/**
 * register_sysctl_table - register a sysctl table hierarchy
 * @table: the top-level table structure
 *
 * Register a sysctl table hierarchy. @table should be a filled in ctl_table
 * array. A completely 0 filled entry terminates the table.
 *
 * See register_sysctl_paths for more details.
 */
struct ctl_table_header *register_sysctl_table(struct ctl_table *table)
{
	static const struct ctl_path null_path[] = { {} };

	return register_sysctl_paths(null_path, table);
}
EXPORT_SYMBOL(register_sysctl_table);

static void put_links(struct ctl_table_header *header)
{
	struct ctl_table_set *root_set = &sysctl_table_root.default_set;
	struct ctl_table_root *root = header->root;
	struct ctl_dir *parent = header->parent;
	struct ctl_dir *core_parent;
	struct ctl_table *entry;

	if (header->set == root_set)
		return;

	core_parent = xlate_dir(root_set, parent);
	if (IS_ERR(core_parent))
		return;

	for (entry = header->ctl_table; entry->procname; entry++) {
		struct ctl_table_header *link_head;
		struct ctl_table *link;
		const char *name = entry->procname;

		link = find_entry(&link_head, core_parent, name, strlen(name));
		if (link &&
		    ((S_ISDIR(link->mode) && S_ISDIR(entry->mode)) ||
		     (S_ISLNK(link->mode) && (link->data == root)))) {
			drop_sysctl_table(link_head);
		}
		else {
			pr_err("sysctl link missing during unregister: ");
			sysctl_print_dir(parent);
			pr_cont("/%s\n", name);
		}
	}
}

static void drop_sysctl_table(struct ctl_table_header *header)
{
	struct ctl_dir *parent = header->parent;

	if (--header->nreg)
		return;

	put_links(header);
	start_unregistering(header);
	if (!--header->count)
		kfree_rcu(header, rcu);

	if (parent)
		drop_sysctl_table(&parent->header);
}

/**
 * unregister_sysctl_table - unregister a sysctl table hierarchy
 * @header: the header returned from register_sysctl_table
 *
 * Unregisters the sysctl table and all children. proc entries may not
 * actually be removed until they are no longer used by anyone.
 */
void unregister_sysctl_table(struct ctl_table_header * header)
{
	int nr_subheaders;
	might_sleep();

	if (header == NULL)
		return;

	nr_subheaders = count_subheaders(header->ctl_table_arg);
	if (unlikely(nr_subheaders > 1)) {
		struct ctl_table_header **subheaders;
		int i;

		subheaders = (struct ctl_table_header **)(header + 1);
		for (i = nr_subheaders -1; i >= 0; i--) {
			struct ctl_table_header *subh = subheaders[i];
			struct ctl_table *table = subh->ctl_table_arg;
			unregister_sysctl_table(subh);
			kfree(table);
		}
		kfree(header);
		return;
	}

	spin_lock(&sysctl_lock);
	drop_sysctl_table(header);
	spin_unlock(&sysctl_lock);
}
EXPORT_SYMBOL(unregister_sysctl_table);

void setup_sysctl_set(struct ctl_table_set *set,
	struct ctl_table_root *root,
	int (*is_seen)(struct ctl_table_set *))
{
	memset(set, 0, sizeof(*set));
	set->is_seen = is_seen;
	init_header(&set->dir.header, root, set, NULL, root_table);
}

void retire_sysctl_set(struct ctl_table_set *set)
{
	WARN_ON(!RB_EMPTY_ROOT(&set->dir.root));
}

int __init proc_sys_init(void)
{
	struct proc_dir_entry *proc_sys_root;

	proc_sys_root = proc_mkdir("sys", NULL);
	proc_sys_root->proc_iops = &proc_sys_dir_operations;
	proc_sys_root->proc_fops = &proc_sys_dir_file_operations;
	proc_sys_root->nlink = 0;

	return sysctl_init();
}