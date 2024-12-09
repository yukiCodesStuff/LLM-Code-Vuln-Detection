		"       %s batch file FILE\n"
		"       %s version\n"
		"\n"
		"       OBJECT := { prog | map | cgroup | perf | net }\n"
		"       " HELP_SPEC_OPTIONS "\n"
		"",
		bin_name, bin_name, bin_name);

	{ "map",	do_map },
	{ "cgroup",	do_cgroup },
	{ "perf",	do_perf },
	{ "net",	do_net },
	{ "version",	do_version },
	{ 0 }
};
