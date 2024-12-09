#define ZEND_OPTIMIZER_INTERNAL_H

#include "zend_ssa.h"

#define ZEND_OP1_LITERAL(opline)		(op_array)->literals[(opline)->op1.constant]
#define ZEND_OP1_JMP_ADDR(opline)		OP_JMP_ADDR(opline, (opline)->op1)
#define ZEND_OP2_LITERAL(opline)		(op_array)->literals[(opline)->op2.constant]
                                     zval          *val);

void zend_optimizer_remove_live_range(zend_op_array *op_array, uint32_t var);
void zend_optimizer_pass1(zend_op_array *op_array, zend_optimizer_ctx *ctx);
void zend_optimizer_pass2(zend_op_array *op_array);
void zend_optimizer_pass3(zend_op_array *op_array);
void zend_optimize_func_calls(zend_op_array *op_array, zend_optimizer_ctx *ctx);
void zend_optimize_cfg(zend_op_array *op_array, zend_optimizer_ctx *ctx);
void zend_optimize_dfa(zend_op_array *op_array, zend_optimizer_ctx *ctx);
int  zend_dfa_analyze_op_array(zend_op_array *op_array, zend_optimizer_ctx *ctx, zend_ssa *ssa, uint32_t *flags);
void zend_dfa_optimize_op_array(zend_op_array *op_array, zend_optimizer_ctx *ctx, zend_ssa *ssa);
void zend_optimize_temporary_variables(zend_op_array *op_array, zend_optimizer_ctx *ctx);
void zend_optimizer_nop_removal(zend_op_array *op_array);
void zend_optimizer_compact_literals(zend_op_array *op_array, zend_optimizer_ctx *ctx);
int zend_optimizer_is_disabled_func(const char *name, size_t len);
zend_function *zend_optimizer_get_called_func(
		zend_script *script, zend_op_array *op_array, zend_op *opline, zend_bool rt_constants);
uint32_t zend_optimizer_classify_function(zend_string *name, uint32_t num_args);
void zend_optimizer_migrate_jump(zend_op_array *op_array, zend_op *new_opline, zend_op *opline);
void zend_optimizer_shift_jump(zend_op_array *op_array, zend_op *opline, uint32_t *shiftlist);

#endif