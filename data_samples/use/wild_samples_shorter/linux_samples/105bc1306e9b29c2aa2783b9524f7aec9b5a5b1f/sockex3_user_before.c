#define PARSE_IP_PROG_FD (prog_fd[0])
#define PROG_ARRAY_FD (map_fd[0])

struct bpf_flow_keys {
	__be32 src;
	__be32 dst;
	union {
		__be32 ports;
	(void) f;

	for (i = 0; i < 5; i++) {
		struct bpf_flow_keys key = {}, next_key;
		struct pair value;

		sleep(1);
		printf("IP     src.port -> dst.port               bytes      packets\n");