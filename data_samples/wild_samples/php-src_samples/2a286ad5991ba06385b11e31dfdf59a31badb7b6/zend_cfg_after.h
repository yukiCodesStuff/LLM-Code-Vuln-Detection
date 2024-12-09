typedef struct _zend_cfg {
	int               blocks_count;       /* number of basic blocks      */
	zend_basic_block *blocks;             /* array of basic blocks       */
	int              *predecessors;
	uint32_t         *map;
	unsigned int      split_at_live_ranges : 1;
	unsigned int      split_at_calls : 1;
	unsigned int      split_at_recv : 1;
	unsigned int      dynamic : 1;        /* accesses varables by name   */
	unsigned int      vararg : 1;         /* uses func_get_args()        */
} zend_cfg;

/* Build Flags */
#define ZEND_RT_CONSTANTS              (1<<31)
#define ZEND_CFG_STACKLESS             (1<<30)
#define ZEND_SSA_DEBUG_LIVENESS        (1<<29)
#define ZEND_SSA_DEBUG_PHI_PLACEMENT   (1<<28)
#define ZEND_SSA_RC_INFERENCE          (1<<27)
#define ZEND_CFG_SPLIT_AT_LIVE_RANGES  (1<<26)
#define ZEND_CFG_NO_ENTRY_PREDECESSORS (1<<25)
#define ZEND_CFG_RECV_ENTRY            (1<<24)
#define ZEND_CALL_TREE                 (1<<23)
#define ZEND_SSA_USE_CV_RESULTS        (1<<22)

#define CRT_CONSTANT_EX(op_array, node, rt_constants) \
	((rt_constants) ? \
		RT_CONSTANT(op_array, (node)) \
	: \
		CT_CONSTANT_EX(op_array, (node).constant) \
	)

#define CRT_CONSTANT(node) \
	CRT_CONSTANT_EX(op_array, node, (build_flags & ZEND_RT_CONSTANTS))

#define RETURN_VALUE_USED(opline) \
	((opline)->result_type != IS_UNUSED)

BEGIN_EXTERN_C()

int  zend_build_cfg(zend_arena **arena, const zend_op_array *op_array, uint32_t build_flags, zend_cfg *cfg, uint32_t *func_flags);
void zend_cfg_remark_reachable_blocks(const zend_op_array *op_array, zend_cfg *cfg);
int  zend_cfg_build_predecessors(zend_arena **arena, zend_cfg *cfg);
int  zend_cfg_compute_dominators_tree(const zend_op_array *op_array, zend_cfg *cfg);
int  zend_cfg_identify_loops(const zend_op_array *op_array, zend_cfg *cfg, uint32_t *flags);

END_EXTERN_C()

#endif /* ZEND_CFG_H */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * indent-tabs-mode: t
 * End:
 */