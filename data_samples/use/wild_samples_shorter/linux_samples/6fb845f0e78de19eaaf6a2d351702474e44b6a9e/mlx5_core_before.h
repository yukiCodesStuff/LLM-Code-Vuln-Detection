		    MLX5_CAP_GEN(dev, lag_master);
}

void mlx5_reload_interface(struct mlx5_core_dev *mdev, int protocol);
void mlx5_lag_update(struct mlx5_core_dev *dev);

enum {