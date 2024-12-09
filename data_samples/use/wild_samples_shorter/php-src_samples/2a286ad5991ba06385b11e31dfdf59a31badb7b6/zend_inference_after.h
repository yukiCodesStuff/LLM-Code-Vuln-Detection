#define MAY_BE_RC1                  (1<<27) /* may be non-reference with refcount == 1 */
#define MAY_BE_RCN                  (1<<28) /* may be non-reference with refcount > 1  */

#define MAY_HAVE_DTOR \
	(MAY_BE_OBJECT|MAY_BE_RESOURCE \
	|MAY_BE_ARRAY_OF_ARRAY|MAY_BE_ARRAY_OF_OBJECT|MAY_BE_ARRAY_OF_RESOURCE)

#define DEFINE_SSA_OP_HAS_RANGE(opN) \
	static zend_always_inline zend_bool _ssa_##opN##_has_range(const zend_op_array *op_array, const zend_ssa *ssa, const zend_op *opline) \
	{ \
                           int                    widening,
                           zend_ssa_var_info     *ret);

int zend_may_throw(const zend_op *opline, zend_op_array *op_array, zend_ssa *ssa);

END_EXTERN_C()

#endif /* ZEND_INFERENCE_H */
