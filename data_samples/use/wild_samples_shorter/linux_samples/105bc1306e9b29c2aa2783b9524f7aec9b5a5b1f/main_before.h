int do_event_pipe(int argc, char **argv);
int do_cgroup(int argc, char **arg);
int do_perf(int argc, char **arg);

int prog_parse_fd(int *argc, char ***argv);
int map_parse_fd(int *argc, char ***argv);
int map_parse_fd_and_info(int *argc, char ***argv, void *info, __u32 *info_len);
 */
int btf_dumper_type(const struct btf_dumper *d, __u32 type_id,
		    const void *data);
#endif