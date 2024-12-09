struct dmar_atsr_unit {
	struct list_head list;		/* list of ATSR units */
	struct acpi_dmar_header *hdr;	/* ACPI header */
	struct dmar_dev_scope *devices;	/* target devices */
	int devices_cnt;		/* target device count */
	u8 include_all:1;		/* include all ports */
};

static LIST_HEAD(dmar_atsr_units);
static LIST_HEAD(dmar_rmrr_units);

#define for_each_rmrr_units(rmrr) \
	list_for_each_entry(rmrr, &dmar_rmrr_units, list)

/* bitmap for indexing intel_iommus */
static int g_num_of_iommus;

static void domain_exit(struct dmar_domain *domain);
static void domain_remove_dev_info(struct dmar_domain *domain);
static void dmar_remove_one_dev_info(struct dmar_domain *domain,
				     struct device *dev);
static void __dmar_remove_one_dev_info(struct device_domain_info *info);
static void domain_context_clear(struct intel_iommu *iommu,
				 struct device *dev);
static int domain_detach_iommu(struct dmar_domain *domain,
			       struct intel_iommu *iommu);

#ifdef CONFIG_INTEL_IOMMU_DEFAULT_ON
int dmar_disabled = 0;
#else
int dmar_disabled = 1;
#endif /*CONFIG_INTEL_IOMMU_DEFAULT_ON*/

int intel_iommu_enabled = 0;
EXPORT_SYMBOL_GPL(intel_iommu_enabled);

static int dmar_map_gfx = 1;
static int dmar_forcedac;
static int intel_iommu_strict;
static int intel_iommu_superpage = 1;
static int intel_iommu_sm = 1;
static int iommu_identity_mapping;

#define IDENTMAP_ALL		1
#define IDENTMAP_GFX		2
#define IDENTMAP_AZALIA		4

#define sm_supported(iommu)	(intel_iommu_sm && ecap_smts((iommu)->ecap))
#define pasid_supported(iommu)	(sm_supported(iommu) &&			\
				 ecap_pasid((iommu)->ecap))

int intel_iommu_gfx_mapped;
EXPORT_SYMBOL_GPL(intel_iommu_gfx_mapped);

#define DUMMY_DEVICE_DOMAIN_INFO ((struct device_domain_info *)(-1))
static DEFINE_SPINLOCK(device_domain_lock);
static LIST_HEAD(device_domain_list);

/*
 * Iterate over elements in device_domain_list and call the specified
 * callback @fn against each element.
 */
int for_each_device_domain(int (*fn)(struct device_domain_info *info,
				     void *data), void *data)
{
	int ret = 0;
	unsigned long flags;
	struct device_domain_info *info;

	spin_lock_irqsave(&device_domain_lock, flags);
	list_for_each_entry(info, &device_domain_list, global) {
		ret = fn(info, data);
		if (ret) {
			spin_unlock_irqrestore(&device_domain_lock, flags);
			return ret;
		}
	}
	spin_unlock_irqrestore(&device_domain_lock, flags);

	return 0;
}
		} else if (!strncmp(str, "sp_off", 6)) {
			pr_info("Disable supported super page\n");
			intel_iommu_superpage = 0;
		} else if (!strncmp(str, "sm_off", 6)) {
			pr_info("Intel-IOMMU: disable scalable mode support\n");
			intel_iommu_sm = 0;
		} else if (!strncmp(str, "tboot_noforce", 13)) {
			printk(KERN_INFO
				"Intel-IOMMU: not forcing on after tboot. This could expose security risk for tboot\n");
			intel_iommu_tboot_noforce = 1;
		}

		str += strcspn(str, ",");
		while (*str == ',')
			str++;
	}
{
	struct iommu_resv_region *entry, *next;

	list_for_each_entry_safe(entry, next, head, list) {
		if (entry->type == IOMMU_RESV_RESERVED)
			kfree(entry);
	}