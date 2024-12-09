static void
nouveau_drm_device_fini(struct drm_device *dev)
{
	struct nouveau_cli *cli, *temp_cli;
	struct nouveau_drm *drm = nouveau_drm(dev);

	if (nouveau_pmops_runtime()) {
		pm_runtime_get_sync(dev->dev);
	nouveau_ttm_fini(drm);
	nouveau_vga_fini(drm);

	/*
	 * There may be existing clients from as-yet unclosed files. For now,
	 * clean them up here rather than deferring until the file is closed,
	 * but this likely not correct if we want to support hot-unplugging
	 * properly.
	 */
	mutex_lock(&drm->clients_lock);
	list_for_each_entry_safe(cli, temp_cli, &drm->clients, head) {
		list_del(&cli->head);
		mutex_lock(&cli->mutex);
		if (cli->abi16)
			nouveau_abi16_fini(cli->abi16);
		mutex_unlock(&cli->mutex);
		nouveau_cli_fini(cli);
		kfree(cli);
	}
	mutex_unlock(&drm->clients_lock);

	nouveau_cli_fini(&drm->client);
	nouveau_cli_fini(&drm->master);
	nvif_parent_dtor(&drm->parent);
	mutex_destroy(&drm->clients_lock);
{
	struct nouveau_cli *cli = nouveau_cli(fpriv);
	struct nouveau_drm *drm = nouveau_drm(dev);
	int dev_index;

	/*
	 * The device is gone, and as it currently stands all clients are
	 * cleaned up in the removal codepath. In the future this may change
	 * so that we can support hot-unplugging, but for now we immediately
	 * return to avoid a double-free situation.
	 */
	if (!drm_dev_enter(dev, &dev_index))
		return;

	pm_runtime_get_sync(dev->dev);

	mutex_lock(&cli->mutex);
	kfree(cli);
	pm_runtime_mark_last_busy(dev->dev);
	pm_runtime_put_autosuspend(dev->dev);
	drm_dev_exit(dev_index);
}

static const struct drm_ioctl_desc
nouveau_ioctls[] = {