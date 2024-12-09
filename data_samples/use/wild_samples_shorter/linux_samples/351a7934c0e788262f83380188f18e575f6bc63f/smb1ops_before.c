#endif /* CIFS_XATTR */
#ifdef CONFIG_CIFS_ACL
	.get_acl = get_cifs_acl,
	.set_acl = set_cifs_acl,
#endif /* CIFS_ACL */
};
