		if (register_netdevice_notifier(&ldev->nb)) {
			ldev->nb.notifier_call = NULL;
			mlx5_core_err(dev, "Failed to register LAG netdev notifier\n");
		}
	}
}

/* Must be called with intf_mutex held */
void mlx5_lag_remove(struct mlx5_core_dev *dev)
{
	struct mlx5_lag *ldev;
	int i;

	ldev = mlx5_lag_dev_get(dev);
	if (!ldev)
		return;

	if (__mlx5_lag_is_active(ldev))
		mlx5_deactivate_lag(ldev);

	mlx5_lag_dev_remove_pf(ldev, dev);

	for (i = 0; i < MLX5_MAX_PORTS; i++)
		if (ldev->pf[i].dev)
			break;

	if (i == MLX5_MAX_PORTS) {
		if (ldev->nb.notifier_call)
			unregister_netdevice_notifier(&ldev->nb);
		cancel_delayed_work_sync(&ldev->bond_work);
		mlx5_lag_dev_free(ldev);
	}