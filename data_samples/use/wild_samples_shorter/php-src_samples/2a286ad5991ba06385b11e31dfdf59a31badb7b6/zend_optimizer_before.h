#define ZEND_OPTIMIZER_PASS_5		(1<<4)   /* CFG based optimization       */
#define ZEND_OPTIMIZER_PASS_6		(1<<5)   /* DFA based optimization       */
#define ZEND_OPTIMIZER_PASS_7		(1<<6)   /* CALL GRAPH optimization      */
#define ZEND_OPTIMIZER_PASS_8		(1<<7)
#define ZEND_OPTIMIZER_PASS_9		(1<<8)   /* TMP VAR usage                */
#define ZEND_OPTIMIZER_PASS_10		(1<<9)   /* NOP removal                 */
#define ZEND_OPTIMIZER_PASS_11		(1<<10)  /* Merge equal constants       */
#define ZEND_OPTIMIZER_PASS_12		(1<<11)  /* Adjust used stack           */
#define ZEND_OPTIMIZER_PASS_13		(1<<12)
#define ZEND_OPTIMIZER_PASS_14		(1<<13)
#define ZEND_OPTIMIZER_PASS_15		(1<<14)  /* Collect constants */
#define ZEND_OPTIMIZER_PASS_16		(1<<15)  /* Inline functions */

#define ZEND_OPTIMIZER_ALL_PASSES	0x7FFFFFFF