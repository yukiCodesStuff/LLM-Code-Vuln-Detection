   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
   | Authors: Dmitry Stogov <dmitry@zend.com>                             |
   +----------------------------------------------------------------------+
*/

#include "php.h"
#include "zend_ssa.h"
#include "zend_dump.h"
#include "zend_inference.h"

static zend_bool dominates(const zend_basic_block *blocks, int a, int b) {
	while (blocks[b].level > blocks[a].level) {
		b = blocks[b].idom;
			ssa_vars[phi->ssa_var].var = phi->var;
			ssa_vars[phi->ssa_var].definition_phi = phi;
			if (phi->pi >= 0) {
				if (phi->sources[0] >= 0) {
					zend_ssa_phi *p = ssa_vars[phi->sources[0]].phi_use_chain;
					while (p && p != phi) {
						p = zend_ssa_next_use_phi(ssa, phi->sources[0], p);
					}
					if (!p) {
						phi->use_chains[0] = ssa_vars[phi->sources[0]].phi_use_chain;
						ssa_vars[phi->sources[0]].phi_use_chain = phi;
					}
				}
				if (phi->has_range_constraint) {
					/* min and max variables can't be used together */
					zend_ssa_range_constraint *constraint = &phi->constraint.range;
				int j;

				for (j = 0; j < ssa->cfg.blocks[i].predecessors_count; j++) {
					if (phi->sources[j] >= 0) {
						zend_ssa_phi *p = ssa_vars[phi->sources[j]].phi_use_chain;
						while (p && p != phi) {
							p = zend_ssa_next_use_phi(ssa, phi->sources[j], p);
						}
						if (!p) {
							phi->use_chains[j] = ssa_vars[phi->sources[j]].phi_use_chain;
							ssa_vars[phi->sources[j]].phi_use_chain = phi;
						}
					}
				}
			}
			phi = phi->next;
}
/* }}} */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4