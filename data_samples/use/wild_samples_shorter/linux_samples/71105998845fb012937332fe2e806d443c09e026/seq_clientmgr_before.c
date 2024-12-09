	struct snd_seq_port_info *info = arg;
	struct snd_seq_client_port *port;
	struct snd_seq_port_callback *callback;

	/* it is not allowed to create the port for an another client */
	if (info->addr.client != client->number)
		return -EPERM;
		return -ENOMEM;

	if (client->type == USER_CLIENT && info->kernel) {
		snd_seq_delete_port(client, port->addr.port);
		return -EINVAL;
	}
	if (client->type == KERNEL_CLIENT) {
		if ((callback = info->kernel) != NULL) {

	snd_seq_set_port_info(port, info);
	snd_seq_system_client_ev_port_start(port->addr.client, port->addr.port);

	return 0;
}
