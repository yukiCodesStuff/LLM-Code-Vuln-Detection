// SPDX-License-Identifier: GPL-2.0
/*
 * sysctl_net_ipv4.c: sysctl interface to net IPV4 subsystem.
 *
 * Begun April 1, 1996, Mike Shaver.
 * Added /proc/sys/net/ipv4 directory entry (empty =) ). [MS]
 */

#include <linux/mm.h>
#include <linux/module.h>
#include <linux/sysctl.h>
#include <linux/igmp.h>
#include <linux/inetdevice.h>
#include <linux/seqlock.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/nsproxy.h>
#include <linux/swap.h>
#include <net/snmp.h>
#include <net/icmp.h>
#include <net/ip.h>
#include <net/route.h>
#include <net/tcp.h>
#include <net/udp.h>
#include <net/cipso_ipv4.h>
#include <net/inet_frag.h>
#include <net/ping.h>
#include <net/protocol.h>
#include <net/netevent.h>

static int zero;
static int one = 1;
static int two = 2;
static int four = 4;
static int thousand = 1000;
static int gso_max_segs = GSO_MAX_SEGS;
static int tcp_retr1_max = 255;
static int ip_local_port_range_min[] = { 1, 1 };
static int ip_local_port_range_max[] = { 65535, 65535 };
static int tcp_adv_win_scale_min = -31;
static int tcp_adv_win_scale_max = 31;
static int tcp_min_snd_mss_min = TCP_MIN_SND_MSS;
static int tcp_min_snd_mss_max = 65535;
static int ip_privileged_port_min;
static int ip_privileged_port_max = 65535;
static int ip_ttl_min = 1;
static int ip_ttl_max = 255;
static int tcp_syn_retries_min = 1;
static int tcp_syn_retries_max = MAX_TCP_SYNCNT;
static int ip_ping_group_range_min[] = { 0, 0 };
static int ip_ping_group_range_max[] = { GID_T_MAX, GID_T_MAX };
static int comp_sack_nr_max = 255;
static u32 u32_max_div_HZ = UINT_MAX / HZ;
static int one_day_secs = 24 * 3600;

DEFINE_STATIC_KEY_FALSE(tcp_rx_skb_cache_key);
EXPORT_SYMBOL(tcp_rx_skb_cache_key);

DEFINE_STATIC_KEY_FALSE(tcp_tx_skb_cache_key);

/* obsolete */
static int sysctl_tcp_low_latency __read_mostly;

/* Update system visible IP port range */
static void set_local_port_range(struct net *net, int range[2])
{
	bool same_parity = !((range[0] ^ range[1]) & 1);

	write_seqlock_bh(&net->ipv4.ip_local_ports.lock);
	if (same_parity && !net->ipv4.ip_local_ports.warned) {
		net->ipv4.ip_local_ports.warned = true;
		pr_err_ratelimited("ip_local_port_range: prefer different parity for start/end values.\n");
	}
	net->ipv4.ip_local_ports.range[0] = range[0];
	net->ipv4.ip_local_ports.range[1] = range[1];
	write_sequnlock_bh(&net->ipv4.ip_local_ports.lock);
}
	{
		.procname	= "tcp_base_mss",
		.data		= &init_net.ipv4.sysctl_tcp_base_mss,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec,
	},
	{
		.procname	= "tcp_min_snd_mss",
		.data		= &init_net.ipv4.sysctl_tcp_min_snd_mss,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec_minmax,
		.extra1		= &tcp_min_snd_mss_min,
		.extra2		= &tcp_min_snd_mss_max,
	},
	{
		.procname	= "tcp_probe_threshold",
		.data		= &init_net.ipv4.sysctl_tcp_probe_threshold,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec,
	},
	{
		.procname	= "tcp_probe_interval",
		.data		= &init_net.ipv4.sysctl_tcp_probe_interval,
		.maxlen		= sizeof(u32),
		.mode		= 0644,
		.proc_handler	= proc_douintvec_minmax,
		.extra2		= &u32_max_div_HZ,
	},
	{
		.procname	= "igmp_link_local_mcast_reports",
		.data		= &init_net.ipv4.sysctl_igmp_llm_reports,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec
	},
	{
		.procname	= "igmp_max_memberships",
		.data		= &init_net.ipv4.sysctl_igmp_max_memberships,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec
	},
	{
		.procname	= "igmp_max_msf",
		.data		= &init_net.ipv4.sysctl_igmp_max_msf,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec
	},
#ifdef CONFIG_IP_MULTICAST
	{
		.procname	= "igmp_qrv",
		.data		= &init_net.ipv4.sysctl_igmp_qrv,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec_minmax,
		.extra1		= &one
	},
#endif
	{
		.procname	= "tcp_congestion_control",
		.data		= &init_net.ipv4.tcp_congestion_control,
		.mode		= 0644,
		.maxlen		= TCP_CA_NAME_MAX,
		.proc_handler	= proc_tcp_congestion_control,
	},
	{
		.procname	= "tcp_keepalive_time",
		.data		= &init_net.ipv4.sysctl_tcp_keepalive_time,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec_jiffies,
	},
	{
		.procname	= "tcp_keepalive_probes",
		.data		= &init_net.ipv4.sysctl_tcp_keepalive_probes,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec
	},
	{
		.procname	= "tcp_keepalive_intvl",
		.data		= &init_net.ipv4.sysctl_tcp_keepalive_intvl,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec_jiffies,
	},
	{
		.procname	= "tcp_syn_retries",
		.data		= &init_net.ipv4.sysctl_tcp_syn_retries,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec_minmax,
		.extra1		= &tcp_syn_retries_min,
		.extra2		= &tcp_syn_retries_max
	},
	{
		.procname	= "tcp_synack_retries",
		.data		= &init_net.ipv4.sysctl_tcp_synack_retries,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec
	},
#ifdef CONFIG_SYN_COOKIES
	{
		.procname	= "tcp_syncookies",
		.data		= &init_net.ipv4.sysctl_tcp_syncookies,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec
	},
#endif
	{
		.procname	= "tcp_reordering",
		.data		= &init_net.ipv4.sysctl_tcp_reordering,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec
	},
	{
		.procname	= "tcp_retries1",
		.data		= &init_net.ipv4.sysctl_tcp_retries1,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec_minmax,
		.extra2		= &tcp_retr1_max
	},
	{
		.procname	= "tcp_retries2",
		.data		= &init_net.ipv4.sysctl_tcp_retries2,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec
	},
	{
		.procname	= "tcp_orphan_retries",
		.data		= &init_net.ipv4.sysctl_tcp_orphan_retries,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec
	},
	{
		.procname	= "tcp_fin_timeout",
		.data		= &init_net.ipv4.sysctl_tcp_fin_timeout,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec_jiffies,
	},
	{
		.procname	= "tcp_notsent_lowat",
		.data		= &init_net.ipv4.sysctl_tcp_notsent_lowat,
		.maxlen		= sizeof(unsigned int),
		.mode		= 0644,
		.proc_handler	= proc_douintvec,
	},
	{
		.procname	= "tcp_tw_reuse",
		.data		= &init_net.ipv4.sysctl_tcp_tw_reuse,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec_minmax,
		.extra1		= &zero,
		.extra2		= &two,
	},
	{
		.procname	= "tcp_max_tw_buckets",
		.data		= &init_net.ipv4.tcp_death_row.sysctl_max_tw_buckets,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec
	},
	{
		.procname	= "tcp_max_syn_backlog",
		.data		= &init_net.ipv4.sysctl_max_syn_backlog,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec
	},
	{
		.procname	= "tcp_fastopen",
		.data		= &init_net.ipv4.sysctl_tcp_fastopen,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec,
	},
	{
		.procname	= "tcp_fastopen_key",
		.mode		= 0600,
		.data		= &init_net.ipv4.sysctl_tcp_fastopen,
		.maxlen		= ((TCP_FASTOPEN_KEY_LENGTH * 2) + 10),
		.proc_handler	= proc_tcp_fastopen_key,
	},
	{
		.procname	= "tcp_fastopen_blackhole_timeout_sec",
		.data		= &init_net.ipv4.sysctl_tcp_fastopen_blackhole_timeout,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= proc_tfo_blackhole_detect_timeout,
		.extra1		= &zero,
	},
#ifdef CONFIG_IP_ROUTE_MULTIPATH
	{
		.procname	= "fib_multipath_use_neigh",
		.data		= &init_net.ipv4.sysctl_fib_multipath_use_neigh,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec_minmax,
		.extra1		= &zero,
		.extra2		= &one,
	},
	{
		.procname	= "fib_multipath_hash_policy",
		.data		= &init_net.ipv4.sysctl_fib_multipath_hash_policy,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= proc_fib_multipath_hash_policy,
		.extra1		= &zero,
		.extra2		= &one,
	},
#endif
	{
		.procname	= "ip_unprivileged_port_start",
		.maxlen		= sizeof(int),
		.data		= &init_net.ipv4.sysctl_ip_prot_sock,
		.mode		= 0644,
		.proc_handler	= ipv4_privileged_ports,
	},
#ifdef CONFIG_NET_L3_MASTER_DEV
	{
		.procname	= "udp_l3mdev_accept",
		.data		= &init_net.ipv4.sysctl_udp_l3mdev_accept,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec_minmax,
		.extra1		= &zero,
		.extra2		= &one,
	},
#endif
	{
		.procname	= "tcp_sack",
		.data		= &init_net.ipv4.sysctl_tcp_sack,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec
	},
	{
		.procname	= "tcp_window_scaling",
		.data		= &init_net.ipv4.sysctl_tcp_window_scaling,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec
	},
	{
		.procname	= "tcp_timestamps",
		.data		= &init_net.ipv4.sysctl_tcp_timestamps,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec
	},
	{
		.procname	= "tcp_early_retrans",
		.data		= &init_net.ipv4.sysctl_tcp_early_retrans,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec_minmax,
		.extra1		= &zero,
		.extra2		= &four,
	},
	{
		.procname	= "tcp_recovery",
		.data		= &init_net.ipv4.sysctl_tcp_recovery,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec,
	},
	{
		.procname       = "tcp_thin_linear_timeouts",
		.data           = &init_net.ipv4.sysctl_tcp_thin_linear_timeouts,
		.maxlen         = sizeof(int),
		.mode           = 0644,
		.proc_handler   = proc_dointvec
	},
	{
		.procname	= "tcp_slow_start_after_idle",
		.data		= &init_net.ipv4.sysctl_tcp_slow_start_after_idle,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec
	},
	{
		.procname	= "tcp_retrans_collapse",
		.data		= &init_net.ipv4.sysctl_tcp_retrans_collapse,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec
	},
	{
		.procname	= "tcp_stdurg",
		.data		= &init_net.ipv4.sysctl_tcp_stdurg,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec
	},
	{
		.procname	= "tcp_rfc1337",
		.data		= &init_net.ipv4.sysctl_tcp_rfc1337,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec
	},
	{
		.procname	= "tcp_abort_on_overflow",
		.data		= &init_net.ipv4.sysctl_tcp_abort_on_overflow,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec
	},
	{
		.procname	= "tcp_fack",
		.data		= &init_net.ipv4.sysctl_tcp_fack,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec
	},
	{
		.procname	= "tcp_max_reordering",
		.data		= &init_net.ipv4.sysctl_tcp_max_reordering,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec
	},
	{
		.procname	= "tcp_dsack",
		.data		= &init_net.ipv4.sysctl_tcp_dsack,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec
	},
	{
		.procname	= "tcp_app_win",
		.data		= &init_net.ipv4.sysctl_tcp_app_win,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec
	},
	{
		.procname	= "tcp_adv_win_scale",
		.data		= &init_net.ipv4.sysctl_tcp_adv_win_scale,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec_minmax,
		.extra1		= &tcp_adv_win_scale_min,
		.extra2		= &tcp_adv_win_scale_max,
	},
	{
		.procname	= "tcp_frto",
		.data		= &init_net.ipv4.sysctl_tcp_frto,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec
	},
	{
		.procname	= "tcp_no_metrics_save",
		.data		= &init_net.ipv4.sysctl_tcp_nometrics_save,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec,
	},
	{
		.procname	= "tcp_moderate_rcvbuf",
		.data		= &init_net.ipv4.sysctl_tcp_moderate_rcvbuf,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec,
	},
	{
		.procname	= "tcp_tso_win_divisor",
		.data		= &init_net.ipv4.sysctl_tcp_tso_win_divisor,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec,
	},
	{
		.procname	= "tcp_workaround_signed_windows",
		.data		= &init_net.ipv4.sysctl_tcp_workaround_signed_windows,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec
	},
	{
		.procname	= "tcp_limit_output_bytes",
		.data		= &init_net.ipv4.sysctl_tcp_limit_output_bytes,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec
	},
	{
		.procname	= "tcp_challenge_ack_limit",
		.data		= &init_net.ipv4.sysctl_tcp_challenge_ack_limit,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec
	},
	{
		.procname	= "tcp_min_tso_segs",
		.data		= &init_net.ipv4.sysctl_tcp_min_tso_segs,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec_minmax,
		.extra1		= &one,
		.extra2		= &gso_max_segs,
	},
	{
		.procname	= "tcp_min_rtt_wlen",
		.data		= &init_net.ipv4.sysctl_tcp_min_rtt_wlen,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec_minmax,
		.extra1		= &zero,
		.extra2		= &one_day_secs
	},
	{
		.procname	= "tcp_autocorking",
		.data		= &init_net.ipv4.sysctl_tcp_autocorking,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec_minmax,
		.extra1		= &zero,
		.extra2		= &one,
	},
	{
		.procname	= "tcp_invalid_ratelimit",
		.data		= &init_net.ipv4.sysctl_tcp_invalid_ratelimit,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec_ms_jiffies,
	},
	{
		.procname	= "tcp_pacing_ss_ratio",
		.data		= &init_net.ipv4.sysctl_tcp_pacing_ss_ratio,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec_minmax,
		.extra1		= &zero,
		.extra2		= &thousand,
	},
	{
		.procname	= "tcp_pacing_ca_ratio",
		.data		= &init_net.ipv4.sysctl_tcp_pacing_ca_ratio,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec_minmax,
		.extra1		= &zero,
		.extra2		= &thousand,
	},
	{
		.procname	= "tcp_wmem",
		.data		= &init_net.ipv4.sysctl_tcp_wmem,
		.maxlen		= sizeof(init_net.ipv4.sysctl_tcp_wmem),
		.mode		= 0644,
		.proc_handler	= proc_dointvec_minmax,
		.extra1		= &one,
	},
	{
		.procname	= "tcp_rmem",
		.data		= &init_net.ipv4.sysctl_tcp_rmem,
		.maxlen		= sizeof(init_net.ipv4.sysctl_tcp_rmem),
		.mode		= 0644,
		.proc_handler	= proc_dointvec_minmax,
		.extra1		= &one,
	},
	{
		.procname	= "tcp_comp_sack_delay_ns",
		.data		= &init_net.ipv4.sysctl_tcp_comp_sack_delay_ns,
		.maxlen		= sizeof(unsigned long),
		.mode		= 0644,
		.proc_handler	= proc_doulongvec_minmax,
	},
	{
		.procname	= "tcp_comp_sack_nr",
		.data		= &init_net.ipv4.sysctl_tcp_comp_sack_nr,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec_minmax,
		.extra1		= &zero,
		.extra2		= &comp_sack_nr_max,
	},
	{
		.procname	= "udp_rmem_min",
		.data		= &init_net.ipv4.sysctl_udp_rmem_min,
		.maxlen		= sizeof(init_net.ipv4.sysctl_udp_rmem_min),
		.mode		= 0644,
		.proc_handler	= proc_dointvec_minmax,
		.extra1		= &one
	},
	{
		.procname	= "udp_wmem_min",
		.data		= &init_net.ipv4.sysctl_udp_wmem_min,
		.maxlen		= sizeof(init_net.ipv4.sysctl_udp_wmem_min),
		.mode		= 0644,
		.proc_handler	= proc_dointvec_minmax,
		.extra1		= &one
	},
	{ }
};

static __net_init int ipv4_sysctl_init_net(struct net *net)
{
	struct ctl_table *table;

	table = ipv4_net_table;
	if (!net_eq(net, &init_net)) {
		int i;

		table = kmemdup(table, sizeof(ipv4_net_table), GFP_KERNEL);
		if (!table)
			goto err_alloc;

		/* Update the variables to point into the current struct net */
		for (i = 0; i < ARRAY_SIZE(ipv4_net_table) - 1; i++)
			table[i].data += (void *)net - (void *)&init_net;
	}

	net->ipv4.ipv4_hdr = register_net_sysctl(net, "net/ipv4", table);
	if (!net->ipv4.ipv4_hdr)
		goto err_reg;

	net->ipv4.sysctl_local_reserved_ports = kzalloc(65536 / 8, GFP_KERNEL);
	if (!net->ipv4.sysctl_local_reserved_ports)
		goto err_ports;

	return 0;

err_ports:
	unregister_net_sysctl_table(net->ipv4.ipv4_hdr);
err_reg:
	if (!net_eq(net, &init_net))
		kfree(table);
err_alloc:
	return -ENOMEM;
}

static __net_exit void ipv4_sysctl_exit_net(struct net *net)
{
	struct ctl_table *table;

	kfree(net->ipv4.sysctl_local_reserved_ports);
	table = net->ipv4.ipv4_hdr->ctl_table_arg;
	unregister_net_sysctl_table(net->ipv4.ipv4_hdr);
	kfree(table);
}

static __net_initdata struct pernet_operations ipv4_sysctl_ops = {
	.init = ipv4_sysctl_init_net,
	.exit = ipv4_sysctl_exit_net,
};

static __init int sysctl_ipv4_init(void)
{
	struct ctl_table_header *hdr;

	hdr = register_net_sysctl(&init_net, "net/ipv4", ipv4_table);
	if (!hdr)
		return -ENOMEM;

	if (register_pernet_subsys(&ipv4_sysctl_ops)) {
		unregister_net_sysctl_table(hdr);
		return -ENOMEM;
	}

	return 0;
}

__initcall(sysctl_ipv4_init);