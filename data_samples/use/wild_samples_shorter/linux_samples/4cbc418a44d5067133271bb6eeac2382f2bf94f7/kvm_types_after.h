
enum kvm_mr_change;

#include <linux/types.h>

/*
 * Address types:
 *
	struct kvm_memory_slot *memslot;
};

struct gfn_to_pfn_cache {
	u64 generation;
	gfn_t gfn;
	kvm_pfn_t pfn;
	bool dirty;
};

#endif /* __KVM_TYPES_H__ */