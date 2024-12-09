static int ip_local_port_range_max[] = { 65535, 65535 };
static int tcp_adv_win_scale_min = -31;
static int tcp_adv_win_scale_max = 31;
static int tcp_min_snd_mss_min = TCP_MIN_SND_MSS;
static int tcp_min_snd_mss_max = 65535;
static int ip_privileged_port_min;
static int ip_privileged_port_max = 65535;
static int ip_ttl_min = 1;
static int ip_ttl_max = 255;
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