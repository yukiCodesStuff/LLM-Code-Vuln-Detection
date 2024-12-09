	 unsigned long arg3, unsigned long arg4, unsigned long arg5)
LSM_HOOK(void, LSM_RET_VOID, task_to_inode, struct task_struct *p,
	 struct inode *inode)
LSM_HOOK(int, 0, ipc_permission, struct kern_ipc_perm *ipcp, short flag)
LSM_HOOK(void, LSM_RET_VOID, ipc_getsecid, struct kern_ipc_perm *ipcp,
	 u32 *secid)
LSM_HOOK(int, 0, msg_msg_alloc_security, struct msg_msg *msg)