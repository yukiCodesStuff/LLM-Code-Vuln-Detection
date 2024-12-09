
static int hb_voltage_change(unsigned int freq)
{
	u32 msg[HB_CPUFREQ_IPC_LEN] = {HB_CPUFREQ_CHANGE_NOTE, freq / 1000000};

	return pl320_ipc_transmit(msg);
}
