		return -EINVAL;
	}

	pr_err("%s: Could not find signal %s\n", __func__, muxname);

	return -ENODEV;
}

int __init omap_mux_get_by_name(const char *muxname,
		return mux_mode;
	}

	return -ENODEV;
}

int __init omap_mux_init_signal(const char *muxname, int val)
	list_for_each_entry(e, &partition->muxmodes, node) {
		struct omap_mux *m = &e->mux;

		(void)debugfs_create_file(m->muxnames[0], S_IWUSR, mux_dbg_dir,
					  m, &omap_mux_dbg_signal_fops);
	}
}

static void __init omap_mux_dbg_init(void)