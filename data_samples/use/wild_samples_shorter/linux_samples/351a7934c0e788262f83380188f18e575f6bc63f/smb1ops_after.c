#endif /* CIFS_XATTR */
#ifdef CONFIG_CIFS_ACL
	.get_acl = get_cifs_acl,
	.get_acl_by_fid = get_cifs_acl_by_fid,
	.set_acl = set_cifs_acl,
#endif /* CIFS_ACL */
};
