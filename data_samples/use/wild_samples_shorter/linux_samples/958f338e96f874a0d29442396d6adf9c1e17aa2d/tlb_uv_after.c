	struct msg_desc msgdesc;

	ack_APIC_irq();
	kvm_set_cpu_l1tf_flush_l1d();
	time_start = get_cycles();

	bcp = &per_cpu(bau_control, smp_processor_id());
	stat = bcp->statp;