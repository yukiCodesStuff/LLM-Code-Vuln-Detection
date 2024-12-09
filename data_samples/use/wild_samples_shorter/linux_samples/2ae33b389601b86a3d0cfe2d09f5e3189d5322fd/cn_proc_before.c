	    (task_active_pid_ns(current) != &init_pid_ns))
		return;

	mc_op = (enum proc_cn_mcast_op *)msg->data;
	switch (*mc_op) {
	case PROC_CN_MCAST_LISTEN:
		atomic_inc(&proc_event_num_listeners);
		err = EINVAL;
		break;
	}
	cn_proc_ack(err, msg->seq, msg->ack);
}

/*