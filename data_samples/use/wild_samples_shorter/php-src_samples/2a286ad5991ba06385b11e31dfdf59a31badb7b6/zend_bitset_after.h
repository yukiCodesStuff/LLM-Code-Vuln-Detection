	} \
} while (0)

static inline int zend_bitset_pop_first(zend_bitset set, uint32_t len) {
	int i = zend_bitset_first(set, len);
	if (i >= 0) {
		zend_bitset_excl(set, i);
	}
	return i;
}

#endif /* _ZEND_BITSET_H_ */

/*
 * Local variables: