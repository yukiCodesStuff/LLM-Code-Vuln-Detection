	if (!test_bit(HCI_QUIRK_NO_SUSPEND_NOTIFIER, &hdev->quirks)) {
		hdev->suspend_notifier.notifier_call = hci_suspend_notifier;
		error = register_pm_notifier(&hdev->suspend_notifier);
		if (error)
			goto err_wqueue;
	}

	queue_work(hdev->req_workqueue, &hdev->power_on);

	idr_init(&hdev->adv_monitors_idr);

	return id;

err_wqueue:
	destroy_workqueue(hdev->workqueue);
	destroy_workqueue(hdev->req_workqueue);
err:
	ida_simple_remove(&hci_index_ida, hdev->id);

	return error;
}
EXPORT_SYMBOL(hci_register_dev);

/* Unregister HCI device */
void hci_unregister_dev(struct hci_dev *hdev)
{
	BT_DBG("%p name %s bus %d", hdev, hdev->name, hdev->bus);

	hci_dev_set_flag(hdev, HCI_UNREGISTER);

	write_lock(&hci_dev_list_lock);
	list_del(&hdev->list);
	write_unlock(&hci_dev_list_lock);

	cancel_work_sync(&hdev->power_on);

	if (!test_bit(HCI_QUIRK_NO_SUSPEND_NOTIFIER, &hdev->quirks)) {
		hci_suspend_clear_tasks(hdev);
		unregister_pm_notifier(&hdev->suspend_notifier);
		cancel_work_sync(&hdev->suspend_prepare);
	}

	hci_dev_do_close(hdev);

	if (!test_bit(HCI_INIT, &hdev->flags) &&
	    !hci_dev_test_flag(hdev, HCI_SETUP) &&
	    !hci_dev_test_flag(hdev, HCI_CONFIG)) {
		hci_dev_lock(hdev);
		mgmt_index_removed(hdev);
		hci_dev_unlock(hdev);
	}

	/* mgmt_index_removed should take care of emptying the
	 * pending list */
	BUG_ON(!list_empty(&hdev->mgmt_pending));

	hci_sock_dev_event(hdev, HCI_DEV_UNREG);

	if (hdev->rfkill) {
		rfkill_unregister(hdev->rfkill);
		rfkill_destroy(hdev->rfkill);
	}

	device_del(&hdev->dev);
	/* Actual cleanup is deferred until hci_cleanup_dev(). */
	hci_dev_put(hdev);
}
	if (hdev->rfkill) {
		rfkill_unregister(hdev->rfkill);
		rfkill_destroy(hdev->rfkill);
	}

	device_del(&hdev->dev);
	/* Actual cleanup is deferred until hci_cleanup_dev(). */
	hci_dev_put(hdev);
}
EXPORT_SYMBOL(hci_unregister_dev);

/* Cleanup HCI device */
void hci_cleanup_dev(struct hci_dev *hdev)
{
	debugfs_remove_recursive(hdev->debugfs);
	kfree_const(hdev->hw_info);
	kfree_const(hdev->fw_info);

	destroy_workqueue(hdev->workqueue);
	destroy_workqueue(hdev->req_workqueue);

	hci_dev_lock(hdev);
	hci_bdaddr_list_clear(&hdev->reject_list);
	hci_bdaddr_list_clear(&hdev->accept_list);
	hci_uuids_clear(hdev);
	hci_link_keys_clear(hdev);
	hci_smp_ltks_clear(hdev);
	hci_smp_irks_clear(hdev);
	hci_remote_oob_data_clear(hdev);
	hci_adv_instances_clear(hdev);
	hci_adv_monitors_clear(hdev);
	hci_bdaddr_list_clear(&hdev->le_accept_list);
	hci_bdaddr_list_clear(&hdev->le_resolv_list);
	hci_conn_params_clear_all(hdev);
	hci_discovery_filter_clear(hdev);
	hci_blocked_keys_clear(hdev);
	hci_dev_unlock(hdev);

	ida_simple_remove(&hci_index_ida, hdev->id);
}

/* Suspend HCI device */
int hci_suspend_dev(struct hci_dev *hdev)
{
	hci_sock_dev_event(hdev, HCI_DEV_SUSPEND);
	return 0;
}
EXPORT_SYMBOL(hci_suspend_dev);

/* Resume HCI device */
int hci_resume_dev(struct hci_dev *hdev)
{
	hci_sock_dev_event(hdev, HCI_DEV_RESUME);
	return 0;
}
EXPORT_SYMBOL(hci_resume_dev);

/* Reset HCI device */
int hci_reset_dev(struct hci_dev *hdev)
{
	static const u8 hw_err[] = { HCI_EV_HARDWARE_ERROR, 0x01, 0x00 };
	struct sk_buff *skb;

	skb = bt_skb_alloc(3, GFP_ATOMIC);
	if (!skb)
		return -ENOMEM;

	hci_skb_pkt_type(skb) = HCI_EVENT_PKT;
	skb_put_data(skb, hw_err, 3);

	bt_dev_err(hdev, "Injecting HCI hardware error event");

	/* Send Hardware Error to upper stack */
	return hci_recv_frame(hdev, skb);
}
EXPORT_SYMBOL(hci_reset_dev);

/* Receive frame from HCI drivers */
int hci_recv_frame(struct hci_dev *hdev, struct sk_buff *skb)
{
	if (!hdev || (!test_bit(HCI_UP, &hdev->flags)
		      && !test_bit(HCI_INIT, &hdev->flags))) {
		kfree_skb(skb);
		return -ENXIO;
	}
{
	debugfs_remove_recursive(hdev->debugfs);
	kfree_const(hdev->hw_info);
	kfree_const(hdev->fw_info);

	destroy_workqueue(hdev->workqueue);
	destroy_workqueue(hdev->req_workqueue);

	hci_dev_lock(hdev);
	hci_bdaddr_list_clear(&hdev->reject_list);
	hci_bdaddr_list_clear(&hdev->accept_list);
	hci_uuids_clear(hdev);
	hci_link_keys_clear(hdev);
	hci_smp_ltks_clear(hdev);
	hci_smp_irks_clear(hdev);
	hci_remote_oob_data_clear(hdev);
	hci_adv_instances_clear(hdev);
	hci_adv_monitors_clear(hdev);
	hci_bdaddr_list_clear(&hdev->le_accept_list);
	hci_bdaddr_list_clear(&hdev->le_resolv_list);
	hci_conn_params_clear_all(hdev);
	hci_discovery_filter_clear(hdev);
	hci_blocked_keys_clear(hdev);
	hci_dev_unlock(hdev);

	ida_simple_remove(&hci_index_ida, hdev->id);
}

/* Suspend HCI device */
int hci_suspend_dev(struct hci_dev *hdev)
{
	hci_sock_dev_event(hdev, HCI_DEV_SUSPEND);
	return 0;
}
EXPORT_SYMBOL(hci_suspend_dev);

/* Resume HCI device */
int hci_resume_dev(struct hci_dev *hdev)
{
	hci_sock_dev_event(hdev, HCI_DEV_RESUME);
	return 0;
}
EXPORT_SYMBOL(hci_resume_dev);

/* Reset HCI device */
int hci_reset_dev(struct hci_dev *hdev)
{
	static const u8 hw_err[] = { HCI_EV_HARDWARE_ERROR, 0x01, 0x00 };
	struct sk_buff *skb;

	skb = bt_skb_alloc(3, GFP_ATOMIC);
	if (!skb)
		return -ENOMEM;

	hci_skb_pkt_type(skb) = HCI_EVENT_PKT;
	skb_put_data(skb, hw_err, 3);

	bt_dev_err(hdev, "Injecting HCI hardware error event");

	/* Send Hardware Error to upper stack */
	return hci_recv_frame(hdev, skb);
}
EXPORT_SYMBOL(hci_reset_dev);

/* Receive frame from HCI drivers */
int hci_recv_frame(struct hci_dev *hdev, struct sk_buff *skb)
{
	if (!hdev || (!test_bit(HCI_UP, &hdev->flags)
		      && !test_bit(HCI_INIT, &hdev->flags))) {
		kfree_skb(skb);
		return -ENXIO;
	}