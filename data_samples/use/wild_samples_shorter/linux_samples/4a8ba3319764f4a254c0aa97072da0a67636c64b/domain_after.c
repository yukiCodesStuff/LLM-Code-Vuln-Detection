 * Returns a valid pointer to struct generic_pm_domain on success or ERR_PTR()
 * on failure.
 */
struct generic_pm_domain *of_genpd_get_from_provider(
					struct of_phandle_args *genpdspec)
{
	struct generic_pm_domain *genpd = ERR_PTR(-ENOENT);
	struct of_genpd_provider *provider;

	return genpd;
}
EXPORT_SYMBOL_GPL(of_genpd_get_from_provider);

/**
 * genpd_dev_pm_detach - Detach a device from its PM domain.
 * @dev: Device to attach.