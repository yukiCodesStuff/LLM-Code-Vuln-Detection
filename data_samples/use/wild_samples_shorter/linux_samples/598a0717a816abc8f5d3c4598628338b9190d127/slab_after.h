static inline bool slab_equal_or_root(struct kmem_cache *s,
				      struct kmem_cache *p)
{
	return s == p;
}

static inline const char *cache_name(struct kmem_cache *s)
{
	 * will also be a constant.
	 */
	if (!memcg_kmem_enabled() &&
	    !IS_ENABLED(CONFIG_SLAB_FREELIST_HARDENED) &&
	    !unlikely(s->flags & SLAB_CONSISTENCY_CHECKS))
		return s;

	page = virt_to_head_page(x);
	cachep = page->slab_cache;
	WARN_ONCE(!slab_equal_or_root(cachep, s),
		  "%s: Wrong slab cache. %s but object is from %s\n",
		  __func__, s->name, cachep->name);
	return cachep;
}

static inline size_t slab_ksize(const struct kmem_cache *s)
{