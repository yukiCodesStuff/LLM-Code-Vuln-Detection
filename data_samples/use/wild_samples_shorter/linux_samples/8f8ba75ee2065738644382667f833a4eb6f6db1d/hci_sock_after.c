	*addr_len = sizeof(*haddr);
	haddr->hci_family = AF_BLUETOOTH;
	haddr->hci_dev    = hdev->id;
	haddr->hci_channel= 0;

	release_sock(sk);
	return 0;
}
		{
			struct hci_filter *f = &hci_pi(sk)->filter;

			memset(&uf, 0, sizeof(uf));
			uf.type_mask = f->type_mask;
			uf.opcode    = f->opcode;
			uf.event_mask[0] = *((u32 *) f->event_mask + 0);
			uf.event_mask[1] = *((u32 *) f->event_mask + 1);