#include "conf_space.h"
#include "conf_space_quirks.h"

bool permissive;
module_param(permissive, bool, 0644);

/* This is where xen_pcibk_read_config_byte, xen_pcibk_read_config_word,
 * xen_pcibk_write_config_word, and xen_pcibk_write_config_byte are created. */