	} else {
		memcpy(dest, src, dest_len);
	}
}
EXPORT_SYMBOL(memcpy_and_pad);

#ifdef CONFIG_FORTIFY_SOURCE
void fortify_panic(const char *name)
{
	pr_emerg("detected buffer overflow in %s\n", name);
	BUG();
}