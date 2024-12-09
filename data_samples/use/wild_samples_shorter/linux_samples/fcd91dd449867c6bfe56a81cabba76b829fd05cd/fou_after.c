	if (!ops || !ops->callbacks.gro_receive)
		goto out_unlock;

	pp = call_gro_receive(ops->callbacks.gro_receive, head, skb);

out_unlock:
	rcu_read_unlock();

	if (WARN_ON_ONCE(!ops || !ops->callbacks.gro_receive))
		goto out_unlock;

	pp = call_gro_receive(ops->callbacks.gro_receive, head, skb);
	flush = 0;

out_unlock:
	rcu_read_unlock();