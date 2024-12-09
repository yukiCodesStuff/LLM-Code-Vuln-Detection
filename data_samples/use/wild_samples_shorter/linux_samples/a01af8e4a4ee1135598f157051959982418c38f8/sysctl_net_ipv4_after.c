static int tcp_retr1_max = 255;
static int ip_local_port_range_min[] = { 1, 1 };
static int ip_local_port_range_max[] = { 65535, 65535 };
static int tcp_adv_win_scale_min = -31;
static int tcp_adv_win_scale_max = 31;

/* Update system visible IP port range */
static void set_local_port_range(int range[2])
{
		.data		= &sysctl_tcp_adv_win_scale,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec_minmax,
		.extra1		= &tcp_adv_win_scale_min,
		.extra2		= &tcp_adv_win_scale_max,
	},
	{
		.procname	= "tcp_tw_reuse",
		.data		= &sysctl_tcp_tw_reuse,