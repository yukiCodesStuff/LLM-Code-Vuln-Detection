hrt_address	debug_buffer_address = (hrt_address)-1;
hrt_vaddress	debug_buffer_ddr_address = (hrt_vaddress)-1;
/* The local copy */
debug_data_t		debug_data;
debug_data_t		*debug_data_ptr = &debug_data;

void debug_buffer_init(const hrt_address addr)
{