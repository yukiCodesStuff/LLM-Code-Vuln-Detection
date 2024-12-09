 * analysis of kexec-tools; if other broken bootloaders initialize a
 * different set of fields we will need to figure out how to disambiguate.
 *
 */
static void sanitize_boot_params(struct boot_params *boot_params)
{
	if (boot_params->sentinel) {
		/*fields in boot_params are not valid, clear them */
		memset(&boot_params->olpc_ofw_header, 0,
		       (char *)&boot_params->alt_mem_k -
			(char *)&boot_params->olpc_ofw_header);
		memset(&boot_params->kbd_status, 0,
		       (char *)&boot_params->hdr -
		       (char *)&boot_params->kbd_status);