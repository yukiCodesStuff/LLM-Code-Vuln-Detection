{
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

void security_ipc_getsecid(struct kern_ipc_perm *ipcp, u32 *secid)
{
	*secid = 0;
	call_void_hook(ipc_getsecid, ipcp, secid);
}

int security_msg_msg_alloc(struct msg_msg *msg)
{
	int rc = lsm_msg_msg_alloc(msg);

	if (unlikely(rc))
		return rc;
	rc = call_int_hook(msg_msg_alloc_security, 0, msg);
	if (unlikely(rc))
		security_msg_msg_free(msg);
	return rc;
}

void security_msg_msg_free(struct msg_msg *msg)
{
	call_void_hook(msg_msg_free_security, msg);
	kfree(msg->security);
	msg->security = NULL;
}

int security_msg_queue_alloc(struct kern_ipc_perm *msq)
{
	int rc = lsm_ipc_alloc(msq);

	if (unlikely(rc))
		return rc;
	rc = call_int_hook(msg_queue_alloc_security, 0, msq);
	if (unlikely(rc))
		security_msg_queue_free(msq);
	return rc;
}

void security_msg_queue_free(struct kern_ipc_perm *msq)
{
	call_void_hook(msg_queue_free_security, msq);
	kfree(msq->security);
	msq->security = NULL;
}

int security_msg_queue_associate(struct kern_ipc_perm *msq, int msqflg)
{
	return call_int_hook(msg_queue_associate, 0, msq, msqflg);
}

int security_msg_queue_msgctl(struct kern_ipc_perm *msq, int cmd)
{
	return call_int_hook(msg_queue_msgctl, 0, msq, cmd);
}

int security_msg_queue_msgsnd(struct kern_ipc_perm *msq,
			       struct msg_msg *msg, int msqflg)
{
	return call_int_hook(msg_queue_msgsnd, 0, msq, msg, msqflg);
}

int security_msg_queue_msgrcv(struct kern_ipc_perm *msq, struct msg_msg *msg,
			       struct task_struct *target, long type, int mode)
{
	return call_int_hook(msg_queue_msgrcv, 0, msq, msg, target, type, mode);
}

int security_shm_alloc(struct kern_ipc_perm *shp)
{
	int rc = lsm_ipc_alloc(shp);

	if (unlikely(rc))
		return rc;
	rc = call_int_hook(shm_alloc_security, 0, shp);
	if (unlikely(rc))
		security_shm_free(shp);
	return rc;
}

void security_shm_free(struct kern_ipc_perm *shp)
{
	call_void_hook(shm_free_security, shp);
	kfree(shp->security);
	shp->security = NULL;
}

int security_shm_associate(struct kern_ipc_perm *shp, int shmflg)
{
	return call_int_hook(shm_associate, 0, shp, shmflg);
}

int security_shm_shmctl(struct kern_ipc_perm *shp, int cmd)
{
	return call_int_hook(shm_shmctl, 0, shp, cmd);
}

int security_shm_shmat(struct kern_ipc_perm *shp, char __user *shmaddr, int shmflg)
{
	return call_int_hook(shm_shmat, 0, shp, shmaddr, shmflg);
}

int security_sem_alloc(struct kern_ipc_perm *sma)
{
	int rc = lsm_ipc_alloc(sma);

	if (unlikely(rc))
		return rc;
	rc = call_int_hook(sem_alloc_security, 0, sma);
	if (unlikely(rc))
		security_sem_free(sma);
	return rc;
}

void security_sem_free(struct kern_ipc_perm *sma)
{
	call_void_hook(sem_free_security, sma);
	kfree(sma->security);
	sma->security = NULL;
}

int security_sem_associate(struct kern_ipc_perm *sma, int semflg)
{
	return call_int_hook(sem_associate, 0, sma, semflg);
}

int security_sem_semctl(struct kern_ipc_perm *sma, int cmd)
{
	return call_int_hook(sem_semctl, 0, sma, cmd);
}

int security_sem_semop(struct kern_ipc_perm *sma, struct sembuf *sops,
			unsigned nsops, int alter)
{
	return call_int_hook(sem_semop, 0, sma, sops, nsops, alter);
}

void security_d_instantiate(struct dentry *dentry, struct inode *inode)
{
	if (unlikely(inode && IS_PRIVATE(inode)))
		return;
	call_void_hook(d_instantiate, dentry, inode);
}
EXPORT_SYMBOL(security_d_instantiate);

int security_getprocattr(struct task_struct *p, const char *lsm, char *name,
				char **value)
{
	struct security_hook_list *hp;

	hlist_for_each_entry(hp, &security_hook_heads.getprocattr, list) {
		if (lsm != NULL && strcmp(lsm, hp->lsm))
			continue;
		return hp->hook.getprocattr(p, name, value);
	}