}


/* create a port, port number is returned (-1 on failure) */
struct snd_seq_client_port *snd_seq_create_port(struct snd_seq_client *client,
						int port)
{
	unsigned long flags;
	snd_use_lock_init(&new_port->use_lock);
	port_subs_info_init(&new_port->c_src);
	port_subs_info_init(&new_port->c_dest);

	num = port >= 0 ? port : 0;
	mutex_lock(&client->ports_mutex);
	write_lock_irqsave(&client->ports_lock, flags);
	list_add_tail(&new_port->list, &p->list);
	client->num_ports++;
	new_port->addr.port = num;	/* store the port number in the port */
	write_unlock_irqrestore(&client->ports_lock, flags);
	mutex_unlock(&client->ports_mutex);
	sprintf(new_port->name, "port-%d", num);

	return new_port;
}
