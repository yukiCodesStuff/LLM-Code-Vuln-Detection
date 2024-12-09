static DEFINE_MUTEX(proto_list_mutex);
static LIST_HEAD(proto_list);

#ifdef CONFIG_MEMCG_KMEM
int mem_cgroup_sockets_init(struct mem_cgroup *memcg, struct cgroup_subsys *ss)
{
	struct proto *proto;