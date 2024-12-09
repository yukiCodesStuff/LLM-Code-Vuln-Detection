	if (json_output) {
		jsonw_null(json_wtr);
		return 0;
	}

	fprintf(stderr,
		"Usage: %s [OPTIONS] OBJECT { COMMAND | help }\n"
		"       %s batch file FILE\n"
		"       %s version\n"
		"\n"
		"       OBJECT := { prog | map | cgroup | perf }\n"
		"       " HELP_SPEC_OPTIONS "\n"
		"",
		bin_name, bin_name, bin_name);

	return 0;
}

static int do_version(int argc, char **argv)
{
	if (json_output) {
		jsonw_start_object(json_wtr);
		jsonw_name(json_wtr, "version");
		jsonw_printf(json_wtr, "\"%s\"", BPFTOOL_VERSION);
		jsonw_end_object(json_wtr);
	} else {
		printf("%s v%s\n", bin_name, BPFTOOL_VERSION);
	}
	return 0;
}
static const struct cmd cmds[] = {
	{ "help",	do_help },
	{ "batch",	do_batch },
	{ "prog",	do_prog },
	{ "map",	do_map },
	{ "cgroup",	do_cgroup },
	{ "perf",	do_perf },
	{ "version",	do_version },
	{ 0 }
};

static int do_batch(int argc, char **argv)
{
	char buf[BATCH_LINE_LEN_MAX], contline[BATCH_LINE_LEN_MAX];
	char *n_argv[BATCH_ARG_NB_MAX];
	unsigned int lines = 0;
	int n_argc;
	FILE *fp;
	char *cp;
	int err;
	int i;

	if (argc < 2) {
		p_err("too few parameters for batch");
		return -1;
	} else if (!is_prefix(*argv, "file")) {
		p_err("expected 'file', got: %s", *argv);
		return -1;
	} else if (argc > 2) {
		p_err("too many parameters for batch");
		return -1;
	}
	NEXT_ARG();

	if (!strcmp(*argv, "-"))
		fp = stdin;
	else
		fp = fopen(*argv, "r");
	if (!fp) {
		p_err("Can't open file (%s): %s", *argv, strerror(errno));
		return -1;
	}

	if (json_output)
		jsonw_start_array(json_wtr);
	while (fgets(buf, sizeof(buf), fp)) {
		cp = strchr(buf, '#');
		if (cp)
			*cp = '\0';

		if (strlen(buf) == sizeof(buf) - 1) {
			errno = E2BIG;
			break;
		}

		/* Append continuation lines if any (coming after a line ending
		 * with '\' in the batch file).
		 */
		while ((cp = strstr(buf, "\\\n")) != NULL) {
			if (!fgets(contline, sizeof(contline), fp) ||
			    strlen(contline) == 0) {
				p_err("missing continuation line on command %d",
				      lines);
				err = -1;
				goto err_close;
			}

			cp = strchr(contline, '#');
			if (cp)
				*cp = '\0';

			if (strlen(buf) + strlen(contline) + 1 > sizeof(buf)) {
				p_err("command %d is too long", lines);
				err = -1;
				goto err_close;
			}
			buf[strlen(buf) - 2] = '\0';
			strcat(buf, contline);
		}

		n_argc = make_args(buf, n_argv, BATCH_ARG_NB_MAX, lines);
		if (!n_argc)
			continue;
		if (n_argc < 0)
			goto err_close;

		if (json_output) {
			jsonw_start_object(json_wtr);
			jsonw_name(json_wtr, "command");
			jsonw_start_array(json_wtr);
			for (i = 0; i < n_argc; i++)
				jsonw_string(json_wtr, n_argv[i]);
			jsonw_end_array(json_wtr);
			jsonw_name(json_wtr, "output");
		}

		err = cmd_select(cmds, n_argc, n_argv, do_help);

		if (json_output)
			jsonw_end_object(json_wtr);

		if (err)
			goto err_close;

		lines++;
	}

	if (errno && errno != ENOENT) {
		p_err("reading batch file failed: %s", strerror(errno));
		err = -1;
	} else {
		p_info("processed %d commands", lines);
		err = 0;
	}
err_close:
	if (fp != stdin)
		fclose(fp);

	if (json_output)
		jsonw_end_array(json_wtr);

	return err;
}

int main(int argc, char **argv)
{
	static const struct option options[] = {
		{ "json",	no_argument,	NULL,	'j' },
		{ "help",	no_argument,	NULL,	'h' },
		{ "pretty",	no_argument,	NULL,	'p' },
		{ "version",	no_argument,	NULL,	'V' },
		{ "bpffs",	no_argument,	NULL,	'f' },
		{ 0 }
	};
	int opt, ret;

	last_do_help = do_help;
	pretty_output = false;
	json_output = false;
	show_pinned = false;
	bin_name = argv[0];

	hash_init(prog_table.table);
	hash_init(map_table.table);

	opterr = 0;
	while ((opt = getopt_long(argc, argv, "Vhpjf",
				  options, NULL)) >= 0) {
		switch (opt) {
		case 'V':
			return do_version(argc, argv);
		case 'h':
			return do_help(argc, argv);
		case 'p':
			pretty_output = true;
			/* fall through */
		case 'j':
			if (!json_output) {
				json_wtr = jsonw_new(stdout);
				if (!json_wtr) {
					p_err("failed to create JSON writer");
					return -1;
				}
				json_output = true;
			}
			jsonw_pretty(json_wtr, pretty_output);
			break;
		case 'f':
			show_pinned = true;
			break;
		default:
			p_err("unrecognized option '%s'", argv[optind - 1]);
			if (json_output)
				clean_and_exit(-1);
			else
				usage();
		}
	}

	argc -= optind;
	argv += optind;
	if (argc < 0)
		usage();

	bfd_init();

	ret = cmd_select(cmds, argc, argv, do_help);

	if (json_output)
		jsonw_destroy(&json_wtr);

	if (show_pinned) {
		delete_pinned_obj_table(&prog_table);
		delete_pinned_obj_table(&map_table);
	}

	return ret;
}