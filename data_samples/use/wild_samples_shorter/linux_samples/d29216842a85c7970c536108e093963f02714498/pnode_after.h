struct mount *copy_tree(struct mount *, struct dentry *, int);
bool is_path_reachable(struct mount *, struct dentry *,
			 const struct path *root);
int count_mounts(struct mnt_namespace *ns, struct mount *mnt);
#endif /* _LINUX_PNODE_H */