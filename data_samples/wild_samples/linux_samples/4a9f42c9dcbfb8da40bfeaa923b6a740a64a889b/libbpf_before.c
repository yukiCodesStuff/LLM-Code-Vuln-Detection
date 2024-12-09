	switch (type) {
	case BPF_PROG_TYPE_SOCKET_FILTER:
	case BPF_PROG_TYPE_SCHED_CLS:
	case BPF_PROG_TYPE_SCHED_ACT:
	case BPF_PROG_TYPE_XDP:
	case BPF_PROG_TYPE_CGROUP_SKB:
	case BPF_PROG_TYPE_CGROUP_SOCK:
	case BPF_PROG_TYPE_LWT_IN:
	case BPF_PROG_TYPE_LWT_OUT:
	case BPF_PROG_TYPE_LWT_XMIT:
	case BPF_PROG_TYPE_LWT_SEG6LOCAL:
	case BPF_PROG_TYPE_SOCK_OPS:
	case BPF_PROG_TYPE_SK_SKB:
	case BPF_PROG_TYPE_CGROUP_DEVICE:
	case BPF_PROG_TYPE_SK_MSG:
	case BPF_PROG_TYPE_CGROUP_SOCK_ADDR:
	case BPF_PROG_TYPE_LIRC_MODE2:
	case BPF_PROG_TYPE_SK_REUSEPORT:
		return false;
	case BPF_PROG_TYPE_UNSPEC:
	case BPF_PROG_TYPE_KPROBE:
	case BPF_PROG_TYPE_TRACEPOINT:
	case BPF_PROG_TYPE_PERF_EVENT:
	case BPF_PROG_TYPE_RAW_TRACEPOINT:
	default:
		return true;
	}
}

static int bpf_object__validate(struct bpf_object *obj, bool needs_kver)
{
	if (needs_kver && obj->kern_version == 0) {
		pr_warning("%s doesn't provide kernel version\n",
			   obj->path);
		return -LIBBPF_ERRNO__KVERSION;
	}
	return 0;
}

static struct bpf_object *
__bpf_object__open(const char *path, void *obj_buf, size_t obj_buf_sz,
		   bool needs_kver)
{
	struct bpf_object *obj;
	int err;

	if (elf_version(EV_CURRENT) == EV_NONE) {
		pr_warning("failed to init libelf for %s\n", path);
		return ERR_PTR(-LIBBPF_ERRNO__LIBELF);
	}

	obj = bpf_object__new(path, obj_buf, obj_buf_sz);
	if (IS_ERR(obj))
		return obj;

	CHECK_ERR(bpf_object__elf_init(obj), err, out);
	CHECK_ERR(bpf_object__check_endianness(obj), err, out);
	CHECK_ERR(bpf_object__elf_collect(obj), err, out);
	CHECK_ERR(bpf_object__collect_reloc(obj), err, out);
	CHECK_ERR(bpf_object__validate(obj, needs_kver), err, out);

	bpf_object__elf_finish(obj);
	return obj;
out:
	bpf_object__close(obj);
	return ERR_PTR(err);
}

struct bpf_object *bpf_object__open_xattr(struct bpf_object_open_attr *attr)
{
	/* param validation */
	if (!attr->file)
		return NULL;

	pr_debug("loading %s\n", attr->file);

	return __bpf_object__open(attr->file, NULL, 0,
				  bpf_prog_type__needs_kver(attr->prog_type));
}

struct bpf_object *bpf_object__open(const char *path)
{
	struct bpf_object_open_attr attr = {
		.file		= path,
		.prog_type	= BPF_PROG_TYPE_UNSPEC,
	};

	return bpf_object__open_xattr(&attr);
}

struct bpf_object *bpf_object__open_buffer(void *obj_buf,
					   size_t obj_buf_sz,
					   const char *name)
{
	char tmp_name[64];

	/* param validation */
	if (!obj_buf || obj_buf_sz <= 0)
		return NULL;

	if (!name) {
		snprintf(tmp_name, sizeof(tmp_name), "%lx-%lx",
			 (unsigned long)obj_buf,
			 (unsigned long)obj_buf_sz);
		tmp_name[sizeof(tmp_name) - 1] = '\0';
		name = tmp_name;
	}
} section_names[] = {
	BPF_PROG_SEC("socket",		BPF_PROG_TYPE_SOCKET_FILTER),
	BPF_PROG_SEC("kprobe/",		BPF_PROG_TYPE_KPROBE),
	BPF_PROG_SEC("kretprobe/",	BPF_PROG_TYPE_KPROBE),
	BPF_PROG_SEC("classifier",	BPF_PROG_TYPE_SCHED_CLS),
	BPF_PROG_SEC("action",		BPF_PROG_TYPE_SCHED_ACT),
	BPF_PROG_SEC("tracepoint/",	BPF_PROG_TYPE_TRACEPOINT),
	BPF_PROG_SEC("raw_tracepoint/",	BPF_PROG_TYPE_RAW_TRACEPOINT),
	BPF_PROG_SEC("xdp",		BPF_PROG_TYPE_XDP),
	BPF_PROG_SEC("perf_event",	BPF_PROG_TYPE_PERF_EVENT),
	BPF_PROG_SEC("cgroup/skb",	BPF_PROG_TYPE_CGROUP_SKB),
	BPF_PROG_SEC("cgroup/sock",	BPF_PROG_TYPE_CGROUP_SOCK),
	BPF_PROG_SEC("cgroup/dev",	BPF_PROG_TYPE_CGROUP_DEVICE),
	BPF_PROG_SEC("lwt_in",		BPF_PROG_TYPE_LWT_IN),
	BPF_PROG_SEC("lwt_out",		BPF_PROG_TYPE_LWT_OUT),
	BPF_PROG_SEC("lwt_xmit",	BPF_PROG_TYPE_LWT_XMIT),
	BPF_PROG_SEC("lwt_seg6local",	BPF_PROG_TYPE_LWT_SEG6LOCAL),
	BPF_PROG_SEC("sockops",		BPF_PROG_TYPE_SOCK_OPS),
	BPF_PROG_SEC("sk_skb",		BPF_PROG_TYPE_SK_SKB),
	BPF_PROG_SEC("sk_msg",		BPF_PROG_TYPE_SK_MSG),
	BPF_PROG_SEC("lirc_mode2",	BPF_PROG_TYPE_LIRC_MODE2),
	BPF_SA_PROG_SEC("cgroup/bind4",	BPF_CGROUP_INET4_BIND),
	BPF_SA_PROG_SEC("cgroup/bind6",	BPF_CGROUP_INET6_BIND),
	BPF_SA_PROG_SEC("cgroup/connect4", BPF_CGROUP_INET4_CONNECT),
	BPF_SA_PROG_SEC("cgroup/connect6", BPF_CGROUP_INET6_CONNECT),
	BPF_SA_PROG_SEC("cgroup/sendmsg4", BPF_CGROUP_UDP4_SENDMSG),
	BPF_SA_PROG_SEC("cgroup/sendmsg6", BPF_CGROUP_UDP6_SENDMSG),
	BPF_S_PROG_SEC("cgroup/post_bind4", BPF_CGROUP_INET4_POST_BIND),
	BPF_S_PROG_SEC("cgroup/post_bind6", BPF_CGROUP_INET6_POST_BIND),
};

#undef BPF_PROG_SEC
#undef BPF_PROG_SEC_FULL
#undef BPF_S_PROG_SEC
#undef BPF_SA_PROG_SEC

int libbpf_prog_type_by_name(const char *name, enum bpf_prog_type *prog_type,
			     enum bpf_attach_type *expected_attach_type)
{
	int i;

	if (!name)
		return -EINVAL;

	for (i = 0; i < ARRAY_SIZE(section_names); i++) {
		if (strncmp(name, section_names[i].sec, section_names[i].len))
			continue;
		*prog_type = section_names[i].prog_type;
		*expected_attach_type = section_names[i].expected_attach_type;
		return 0;
	}
	return -EINVAL;
}