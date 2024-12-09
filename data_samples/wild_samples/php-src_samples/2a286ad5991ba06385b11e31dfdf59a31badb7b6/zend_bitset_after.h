			while (x != Z_UL(0)) {
				x = x >> Z_UL(1);
				j++;
			}
			return j;
		}
	}
	return -1; /* empty set */
}

#define ZEND_BITSET_FOREACH(set, len, bit) do { \
	zend_bitset _set = (set); \
	uint32_t _i, _len = (len); \
	for (_i = 0; _i < _len; _i++) { \
		zend_ulong _x = _set[_i]; \
		if (_x) { \
			(bit) = ZEND_BITSET_ELM_SIZE * 8 * _i; \
			for (; _x != 0; _x >>= Z_UL(1), (bit)++) { \
				if (!(_x & Z_UL(1))) continue;

#define ZEND_BITSET_REVERSE_FOREACH(set, len, bit) do { \
	zend_bitset _set = (set); \
	uint32_t _i = (len); \
	zend_ulong _test = Z_UL(1) << (ZEND_BITSET_ELM_SIZE * 8 - 1); \
	while (_i-- > 0) { \
		zend_ulong _x = _set[_i]; \
		if (_x) { \
			(bit) = ZEND_BITSET_ELM_SIZE * 8 * (_i + 1) - 1; \
			for (; _x != 0; _x <<= Z_UL(1), (bit)--) { \
				if (!(_x & _test)) continue; \

#define ZEND_BITSET_FOREACH_END() \
			} \
		} \
	} \
} while (0)

static inline int zend_bitset_pop_first(zend_bitset set, uint32_t len) {
	int i = zend_bitset_first(set, len);
	if (i >= 0) {
		zend_bitset_excl(set, i);
	}
	return i;
}