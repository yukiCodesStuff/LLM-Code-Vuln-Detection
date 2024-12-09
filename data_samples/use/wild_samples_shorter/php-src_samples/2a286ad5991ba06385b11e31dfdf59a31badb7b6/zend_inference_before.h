#define MAY_BE_RC1                  (1<<27) /* may be non-reference with refcount == 1 */
#define MAY_BE_RCN                  (1<<28) /* may be non-reference with refcount > 1  */


#define DEFINE_SSA_OP_HAS_RANGE(opN) \
	static zend_always_inline zend_bool _ssa_##opN##_has_range(const zend_op_array *op_array, const zend_ssa *ssa, const zend_op *opline) \
	{ \
                           int                    widening,
                           zend_ssa_var_info     *ret);

END_EXTERN_C()

#endif /* ZEND_INFERENCE_H */
