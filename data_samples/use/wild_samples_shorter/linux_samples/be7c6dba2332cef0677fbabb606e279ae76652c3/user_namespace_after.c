			kuid_t uid = make_kuid(ns->parent, id);
			if (uid_eq(uid, file->f_cred->fsuid))
				return true;
		}
	}

	/* Allow anyone to set a mapping that doesn't require privilege */