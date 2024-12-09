static DEFINE_MUTEX(proto_list_mutex);
static LIST_HEAD(proto_list);

/**
 * sk_ns_capable - General socket capability test
 * @sk: Socket to use a capability on or through
 * @user_ns: The user namespace of the capability to use
 * @cap: The capability to use
 *
 * Test to see if the opener of the socket had when the socket was
 * created and the current process has the capability @cap in the user
 * namespace @user_ns.
 */
bool sk_ns_capable(const struct sock *sk,
		   struct user_namespace *user_ns, int cap)
{
	return file_ns_capable(sk->sk_socket->file, user_ns, cap) &&
		ns_capable(user_ns, cap);
}
EXPORT_SYMBOL(sk_ns_capable);

/**
 * sk_capable - Socket global capability test
 * @sk: Socket to use a capability on or through
 * @cap: The global capbility to use
 *
 * Test to see if the opener of the socket had when the socket was
 * created and the current process has the capability @cap in all user
 * namespaces.
 */
bool sk_capable(const struct sock *sk, int cap)
{
	return sk_ns_capable(sk, &init_user_ns, cap);
}
EXPORT_SYMBOL(sk_capable);

/**
 * sk_net_capable - Network namespace socket capability test
 * @sk: Socket to use a capability on or through
 * @cap: The capability to use
 *
 * Test to see if the opener of the socket had when the socke was created
 * and the current process has the capability @cap over the network namespace
 * the socket is a member of.
 */
bool sk_net_capable(const struct sock *sk, int cap)
{
	return sk_ns_capable(sk, sock_net(sk)->user_ns, cap);
}
EXPORT_SYMBOL(sk_net_capable);


#ifdef CONFIG_MEMCG_KMEM
int mem_cgroup_sockets_init(struct mem_cgroup *memcg, struct cgroup_subsys *ss)
{
	struct proto *proto;