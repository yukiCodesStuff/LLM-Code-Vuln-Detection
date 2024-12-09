	HIVE_INPUT_SWITCH_SELECT_STR_TO_MEM};

/* MW Should be part of system_global.h, where we have the main enumeration */
const bool HIVE_IF_BIN_COPY[N_INPUT_FORMATTER_ID] = {
	false, false, false, true};

void input_formatter_rst(
	const input_formatter_ID_t		ID)
{