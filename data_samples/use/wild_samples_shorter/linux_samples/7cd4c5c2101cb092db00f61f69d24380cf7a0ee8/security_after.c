	call_void_hook(task_to_inode, p, inode);
}

int security_create_user_ns(const struct cred *cred)
{
	return call_int_hook(userns_create, 0, cred);
}

int security_ipc_permission(struct kern_ipc_perm *ipcp, short flag)
{
	return call_int_hook(ipc_permission, 0, ipcp, flag);
}