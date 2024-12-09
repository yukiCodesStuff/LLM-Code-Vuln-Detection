	buf += "	.release_cmd			= " + fabric_mod_name + "_release_cmd,\n"
	buf += "	.shutdown_session		= " + fabric_mod_name + "_shutdown_session,\n"
	buf += "	.close_session			= " + fabric_mod_name + "_close_session,\n"
	buf += "	.sess_get_index			= " + fabric_mod_name + "_sess_get_index,\n"
	buf += "	.sess_get_initiator_sid		= NULL,\n"
	buf += "	.write_pending			= " + fabric_mod_name + "_write_pending,\n"
	buf += "	.write_pending_status		= " + fabric_mod_name + "_write_pending_status,\n"
	buf += "	.queue_data_in			= " + fabric_mod_name + "_queue_data_in,\n"
	buf += "	.queue_status			= " + fabric_mod_name + "_queue_status,\n"
	buf += "	.queue_tm_rsp			= " + fabric_mod_name + "_queue_tm_rsp,\n"
	buf += "	.aborted_task			= " + fabric_mod_name + "_aborted_task,\n"
	buf += "	/*\n"
	buf += "	 * Setup function pointers for generic logic in target_core_fabric_configfs.c\n"
	buf += "	 */\n"
	buf += "	.fabric_make_wwn		= " + fabric_mod_name + "_make_" + fabric_mod_port + ",\n"
	buf += "	/*\n"
	buf += "	 * Register the top level struct config_item_type with TCM core\n"
	buf += "	 */\n"
	buf += "	fabric = target_fabric_configfs_init(THIS_MODULE, \"" + fabric_mod_name + "\");\n"
	buf += "	if (IS_ERR(fabric)) {\n"
	buf += "		printk(KERN_ERR \"target_fabric_configfs_init() failed\\n\");\n"
	buf += "		return PTR_ERR(fabric);\n"
	buf += "	}\n"
		if re.search('get_fabric_name', fo):
			buf += "char *" + fabric_mod_name + "_get_fabric_name(void)\n"
			buf += "{\n"
			buf += "	return \"" + fabric_mod_name + "\";\n"
			buf += "}\n\n"
			bufi += "char *" + fabric_mod_name + "_get_fabric_name(void);\n"
			continue

			buf += "}\n\n"
			bufi += "void " + fabric_mod_name + "_close_session(struct se_session *);\n"

		if re.search('sess_get_index\)\(', fo):
			buf += "u32 " + fabric_mod_name + "_sess_get_index(struct se_session *se_sess)\n"
			buf += "{\n"
			buf += "	return 0;\n"
			bufi += "int " + fabric_mod_name + "_queue_status(struct se_cmd *);\n"

		if re.search('queue_tm_rsp\)\(', fo):
			buf += "void " + fabric_mod_name + "_queue_tm_rsp(struct se_cmd *se_cmd)\n"
			buf += "{\n"
			buf += "	return;\n"
			buf += "}\n\n"
			bufi += "void " + fabric_mod_name + "_queue_tm_rsp(struct se_cmd *);\n"

		if re.search('aborted_task\)\(', fo):
			buf += "void " + fabric_mod_name + "_aborted_task(struct se_cmd *se_cmd)\n"
			buf += "{\n"
			buf += "	return;\n"
			buf += "}\n\n"
			bufi += "void " + fabric_mod_name + "_aborted_task(struct se_cmd *);\n"

	ret = p.write(buf)
	if ret:
		tcm_mod_err("Unable to write f: " + f)
	tcm_mod_build_kbuild(fabric_mod_dir, fabric_mod_name)
	tcm_mod_build_kconfig(fabric_mod_dir, fabric_mod_name)

	input = raw_input("Would you like to add " + fabric_mod_name + " to drivers/target/Makefile..? [yes,no]: ")
	if input == "yes" or input == "y":
		tcm_mod_add_kbuild(tcm_dir, fabric_mod_name)

	input = raw_input("Would you like to add " + fabric_mod_name + " to drivers/target/Kconfig..? [yes,no]: ")
	if input == "yes" or input == "y":
		tcm_mod_add_kconfig(tcm_dir, fabric_mod_name)

	return