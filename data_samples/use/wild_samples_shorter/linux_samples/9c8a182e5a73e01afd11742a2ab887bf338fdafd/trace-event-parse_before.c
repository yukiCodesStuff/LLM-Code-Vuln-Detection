		printk = strdup(fmt+1);
		line = strtok_r(NULL, "\n", &next);
		tep_register_print_string(pevent, printk, addr);
	}
}

void parse_saved_cmdline(struct tep_handle *pevent,