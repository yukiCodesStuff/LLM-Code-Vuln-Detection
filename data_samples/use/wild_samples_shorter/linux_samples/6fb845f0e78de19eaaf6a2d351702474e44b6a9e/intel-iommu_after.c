static int dmar_forcedac;
static int intel_iommu_strict;
static int intel_iommu_superpage = 1;
static int intel_iommu_sm;
static int iommu_identity_mapping;

#define IDENTMAP_ALL		1
#define IDENTMAP_GFX		2
		} else if (!strncmp(str, "sp_off", 6)) {
			pr_info("Disable supported super page\n");
			intel_iommu_superpage = 0;
		} else if (!strncmp(str, "sm_on", 5)) {
			pr_info("Intel-IOMMU: scalable mode supported\n");
			intel_iommu_sm = 1;
		} else if (!strncmp(str, "tboot_noforce", 13)) {
			printk(KERN_INFO
				"Intel-IOMMU: not forcing on after tboot. This could expose security risk for tboot\n");
			intel_iommu_tboot_noforce = 1;
	struct iommu_resv_region *entry, *next;

	list_for_each_entry_safe(entry, next, head, list) {
		if (entry->type == IOMMU_RESV_MSI)
			kfree(entry);
	}
}
