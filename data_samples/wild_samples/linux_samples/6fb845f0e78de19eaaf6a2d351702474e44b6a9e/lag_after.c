		if (register_netdevice_notifier(&ldev->nb)) {
			ldev->nb.notifier_call = NULL;
			mlx5_core_err(dev, "Failed to register LAG netdev notifier\n");
		}
	}
}

int mlx5_lag_get_pf_num(struct mlx5_core_dev *dev, int *pf_num)
{
	struct mlx5_lag *ldev;
	int n;

	ldev = mlx5_lag_dev_get(dev);
	if (!ldev) {
		mlx5_core_warn(dev, "no lag device, can't get pf num\n");
		return -EINVAL;
	}