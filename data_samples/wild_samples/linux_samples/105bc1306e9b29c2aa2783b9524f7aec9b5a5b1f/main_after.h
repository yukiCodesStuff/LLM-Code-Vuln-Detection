struct cmd {
	const char *cmd;
	int (*func)(int argc, char **argv);
};

int cmd_select(const struct cmd *cmds, int argc, char **argv,
	       int (*help)(int argc, char **argv));

int get_fd_type(int fd);
const char *get_fd_type_name(enum bpf_obj_type type);
char *get_fdinfo(int fd, const char *key);
int open_obj_pinned(char *path);
int open_obj_pinned_any(char *path, enum bpf_obj_type exp_type);
int do_pin_any(int argc, char **argv, int (*get_fd_by_id)(__u32));
int do_pin_fd(int fd, const char *name);

int do_prog(int argc, char **arg);
int do_map(int argc, char **arg);
int do_event_pipe(int argc, char **argv);
int do_cgroup(int argc, char **arg);
int do_perf(int argc, char **arg);
int do_net(int argc, char **arg);

int prog_parse_fd(int *argc, char ***argv);
int map_parse_fd(int *argc, char ***argv);
int map_parse_fd_and_info(int *argc, char ***argv, void *info, __u32 *info_len);

void disasm_print_insn(unsigned char *image, ssize_t len, int opcodes,
		       const char *arch);
void print_data_json(uint8_t *data, size_t len);
void print_hex_data_json(uint8_t *data, size_t len);

unsigned int get_page_size(void);
unsigned int get_possible_cpus(void);
const char *ifindex_to_bfd_name_ns(__u32 ifindex, __u64 ns_dev, __u64 ns_ino);

struct btf_dumper {
	const struct btf *btf;
	json_writer_t *jw;
	bool is_plain_text;
};

/* btf_dumper_type - print data along with type information
 * @d: an instance containing context for dumping types
 * @type_id: index in btf->types array. this points to the type to be dumped
 * @data: pointer the actual data, i.e. the values to be printed
 *
 * Returns zero on success and negative error code otherwise
 */
int btf_dumper_type(const struct btf_dumper *d, __u32 type_id,
		    const void *data);

struct nlattr;
struct ifinfomsg;
struct tcmsg;
int do_xdp_dump(struct ifinfomsg *ifinfo, struct nlattr **tb);
int do_filter_dump(struct tcmsg *ifinfo, struct nlattr **tb, const char *kind,
		   const char *devname, int ifindex);
#endif
struct btf_dumper {
	const struct btf *btf;
	json_writer_t *jw;
	bool is_plain_text;
};

/* btf_dumper_type - print data along with type information
 * @d: an instance containing context for dumping types
 * @type_id: index in btf->types array. this points to the type to be dumped
 * @data: pointer the actual data, i.e. the values to be printed
 *
 * Returns zero on success and negative error code otherwise
 */
int btf_dumper_type(const struct btf_dumper *d, __u32 type_id,
		    const void *data);

struct nlattr;
struct ifinfomsg;
struct tcmsg;
int do_xdp_dump(struct ifinfomsg *ifinfo, struct nlattr **tb);
int do_filter_dump(struct tcmsg *ifinfo, struct nlattr **tb, const char *kind,
		   const char *devname, int ifindex);
#endif