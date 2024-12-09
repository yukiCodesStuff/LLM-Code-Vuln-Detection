		if (!addr_str) {
			pr_warning("printk format with empty entry");
			break;
		}
		addr = strtoull(addr_str, NULL, 16);
		/* fmt still has a space, skip it */
		printk = strdup(fmt+1);
		line = strtok_r(NULL, "\n", &next);
		tep_register_print_string(pevent, printk, addr);
	}
}

void parse_saved_cmdline(struct tep_handle *pevent,
			 char *file, unsigned int size __maybe_unused)
{
	char comm[17]; /* Max comm length in the kernel is 16. */
	char *line;
	char *next = NULL;
	int pid;

	line = strtok_r(file, "\n", &next);
	while (line) {
		if (sscanf(line, "%d %16s", &pid, comm) == 2)
			tep_register_comm(pevent, comm, pid);
		line = strtok_r(NULL, "\n", &next);
	}
}

int parse_ftrace_file(struct tep_handle *pevent, char *buf, unsigned long size)
{
	return tep_parse_event(pevent, buf, size, "ftrace");
}

int parse_event_file(struct tep_handle *pevent,
		     char *buf, unsigned long size, char *sys)
{
	return tep_parse_event(pevent, buf, size, sys);
}

struct tep_event_format *trace_find_next_event(struct tep_handle *pevent,
					       struct tep_event_format *event)
{
	static int idx;

	if (!pevent || !pevent->events)
		return NULL;

	if (!event) {
		idx = 0;
		return pevent->events[0];
	}

	if (idx < pevent->nr_events && event == pevent->events[idx]) {
		idx++;
		if (idx == pevent->nr_events)
			return NULL;
		return pevent->events[idx];
	}

	for (idx = 1; idx < pevent->nr_events; idx++) {
		if (event == pevent->events[idx - 1])
			return pevent->events[idx];
	}
	return NULL;
}

struct flag {
	const char *name;
	unsigned long long value;
};

static const struct flag flags[] = {
	{ "HI_SOFTIRQ", 0 },
	{ "TIMER_SOFTIRQ", 1 },
	{ "NET_TX_SOFTIRQ", 2 },
	{ "NET_RX_SOFTIRQ", 3 },
	{ "BLOCK_SOFTIRQ", 4 },
	{ "IRQ_POLL_SOFTIRQ", 5 },
	{ "TASKLET_SOFTIRQ", 6 },
	{ "SCHED_SOFTIRQ", 7 },
	{ "HRTIMER_SOFTIRQ", 8 },
	{ "RCU_SOFTIRQ", 9 },

	{ "HRTIMER_NORESTART", 0 },
	{ "HRTIMER_RESTART", 1 },
};

unsigned long long eval_flag(const char *flag)
{
	int i;

	/*
	 * Some flags in the format files do not get converted.
	 * If the flag is not numeric, see if it is something that
	 * we already know about.
	 */
	if (isdigit(flag[0]))
		return strtoull(flag, NULL, 0);

	for (i = 0; i < (int)(sizeof(flags)/sizeof(flags[0])); i++)
		if (strcmp(flags[i].name, flag) == 0)
			return flags[i].value;

	return 0;
}