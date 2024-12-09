	else
		i = -EIO;
	tty_ldisc_deref(ld);

	return i;
}

void tty_write_unlock(struct tty_struct *tty)
			break;
		cond_resched();
	}
	if (written)
		ret = written;
out:
	tty_write_unlock(tty);
	return ret;
}