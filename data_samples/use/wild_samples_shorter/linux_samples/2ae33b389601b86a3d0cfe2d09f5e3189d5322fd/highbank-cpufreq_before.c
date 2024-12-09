
static int hb_voltage_change(unsigned int freq)
{
	int i;
	u32 msg[HB_CPUFREQ_IPC_LEN];

	msg[0] = HB_CPUFREQ_CHANGE_NOTE;
	msg[1] = freq / 1000000;
	for (i = 2; i < HB_CPUFREQ_IPC_LEN; i++)
		msg[i] = 0;

	return pl320_ipc_transmit(msg);
}
