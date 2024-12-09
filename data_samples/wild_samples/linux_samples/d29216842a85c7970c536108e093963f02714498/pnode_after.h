{
	mnt->mnt.mnt_flags &= ~MNT_SHARED_MASK;
	mnt->mnt.mnt_flags |= MNT_SHARED;
}

void change_mnt_propagation(struct mount *, int);
int propagate_mnt(struct mount *, struct mountpoint *, struct mount *,
		struct hlist_head *);
int propagate_umount(struct list_head *);
int propagate_mount_busy(struct mount *, int);
void propagate_mount_unlock(struct mount *);
void mnt_release_group_id(struct mount *);
int get_dominating_id(struct mount *mnt, const struct path *root);
unsigned int mnt_get_count(struct mount *mnt);
void mnt_set_mountpoint(struct mount *, struct mountpoint *,
			struct mount *);
struct mount *copy_tree(struct mount *, struct dentry *, int);
bool is_path_reachable(struct mount *, struct dentry *,
			 const struct path *root);
int count_mounts(struct mnt_namespace *ns, struct mount *mnt);
#endif /* _LINUX_PNODE_H */