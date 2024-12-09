	else
		i = -EIO;
	tty_ldisc_deref(ld);
	if (i > 0)
		inode->i_atime = current_fs_time(inode->i_sb);
	return i;
}

void tty_write_unlock(struct tty_struct *tty)
			break;
		cond_resched();
	}
	if (written) {
		struct inode *inode = file->f_path.dentry->d_inode;
		inode->i_mtime = current_fs_time(inode->i_sb);
		ret = written;
	}
out:
	tty_write_unlock(tty);
	return ret;
}