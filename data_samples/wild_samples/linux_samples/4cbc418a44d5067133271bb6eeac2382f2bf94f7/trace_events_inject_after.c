{
	struct ftrace_event_field *field;
	unsigned long irq_flags;
	void *entry = NULL;
	int entry_size;
	u64 val = 0;
	int len;

	entry = trace_alloc_entry(call, &entry_size);
	*pentry = entry;
	if (!entry)
		return -ENOMEM;

	local_save_flags(irq_flags);
	tracing_generic_entry_update(entry, call->event.type, irq_flags,
				     preempt_count());

	while ((len = parse_field(str, call, &field, &val)) > 0) {
		if (is_function_field(field))
			return -EINVAL;

		if (is_string_field(field)) {
			char *addr = (char *)(unsigned long) val;

			if (field->filter_type == FILTER_STATIC_STRING) {
				strlcpy(entry + field->offset, addr, field->size);
			} else if (field->filter_type == FILTER_DYN_STRING) {
				int str_len = strlen(addr) + 1;
				int str_loc = entry_size & 0xffff;
				u32 *str_item;

				entry_size += str_len;
				*pentry = krealloc(entry, entry_size, GFP_KERNEL);
				if (!*pentry) {
					kfree(entry);
					return -ENOMEM;
				}
				entry = *pentry;

				strlcpy(entry + (entry_size - str_len), addr, str_len);
				str_item = (u32 *)(entry + field->offset);
				*str_item = (str_len << 16) | str_loc;
			} else {
				char **paddr;

				paddr = (char **)(entry + field->offset);
				*paddr = INJECT_STRING;
			}
		} else {
			switch (field->size) {
			case 1: {
				u8 tmp = (u8) val;

				memcpy(entry + field->offset, &tmp, 1);
				break;
			}
			case 2: {
				u16 tmp = (u16) val;

				memcpy(entry + field->offset, &tmp, 2);
				break;
			}
			case 4: {
				u32 tmp = (u32) val;

				memcpy(entry + field->offset, &tmp, 4);
				break;
			}
			case 8:
				memcpy(entry + field->offset, &val, 8);
				break;
			default:
				return -EINVAL;
			}
		}

		str += len;
	}

	if (len < 0)
		return len;

	return entry_size;
}

static ssize_t
event_inject_write(struct file *filp, const char __user *ubuf, size_t cnt,
		   loff_t *ppos)
{
	struct trace_event_call *call;
	struct trace_event_file *file;
	int err = -ENODEV, size;
	void *entry = NULL;
	char *buf;

	if (cnt >= PAGE_SIZE)
		return -EINVAL;

	buf = memdup_user_nul(ubuf, cnt);
	if (IS_ERR(buf))
		return PTR_ERR(buf);
	strim(buf);

	mutex_lock(&event_mutex);
	file = event_file_data(filp);
	if (file) {
		call = file->event_call;
		size = parse_entry(buf, call, &entry);
		if (size < 0)
			err = size;
		else
			err = trace_inject_entry(file, entry, size);
	}
	mutex_unlock(&event_mutex);

	kfree(entry);
	kfree(buf);

	if (err < 0)
		return err;

	*ppos += err;
	return cnt;
}

static ssize_t
event_inject_read(struct file *file, char __user *buf, size_t size,
		  loff_t *ppos)
{
	return -EPERM;
}

const struct file_operations event_inject_fops = {
	.open = tracing_open_generic,
	.read = event_inject_read,
	.write = event_inject_write,
};