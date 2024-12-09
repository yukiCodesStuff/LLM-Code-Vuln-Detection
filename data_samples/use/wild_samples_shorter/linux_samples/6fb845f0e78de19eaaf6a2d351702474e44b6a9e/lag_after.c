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

	for (n = 0; n < MLX5_MAX_PORTS; n++)
		if (ldev->pf[n].dev == dev) {
			*pf_num = n;
			return 0;
		}

	mlx5_core_warn(dev, "wasn't able to locate pf in the lag device\n");
	return -EINVAL;
}

/* Must be called with intf_mutex held */
void mlx5_lag_remove(struct mlx5_core_dev *dev)
{
	struct mlx5_lag *ldev;