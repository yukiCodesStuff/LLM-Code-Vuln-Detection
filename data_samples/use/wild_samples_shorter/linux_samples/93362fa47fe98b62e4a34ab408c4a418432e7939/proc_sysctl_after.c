	ctl_dir = container_of(head, struct ctl_dir, header);

	if (!dir_emit_dots(file, ctx))
		goto out;

	pos = 2;

	for (first_entry(ctl_dir, &h, &entry); h; next_entry(&h, &entry)) {
			break;
		}
	}
out:
	sysctl_head_finish(head);
	return 0;
}
