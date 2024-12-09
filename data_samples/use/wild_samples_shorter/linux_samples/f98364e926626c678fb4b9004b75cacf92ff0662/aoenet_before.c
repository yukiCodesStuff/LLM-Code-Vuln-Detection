			pr_warn("aoe: packet could not be sent on %s.  %s\n",
				ifp ? ifp->name : "netif",
				"consider increasing tx_queue_len");
		spin_lock_irq(&txlock);
	}
	return 0;
}