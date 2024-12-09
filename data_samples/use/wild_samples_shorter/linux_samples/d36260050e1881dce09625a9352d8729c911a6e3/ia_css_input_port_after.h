 * more details.
 */

/* For MIPI_PORT0_ID to MIPI_PORT2_ID */
#include "system_global.h"

#ifndef __IA_CSS_INPUT_PORT_H
#define __IA_CSS_INPUT_PORT_H

/* @file
 * This file contains information about the possible input ports for CSS
 */

/* Backward compatible for CSS API 2.0 only
 *  TO BE REMOVED when all drivers move to CSS	API 2.1
 */
#define	IA_CSS_CSI2_PORT_4LANE MIPI_PORT0_ID
#define	IA_CSS_CSI2_PORT_1LANE MIPI_PORT1_ID
#define	IA_CSS_CSI2_PORT_2LANE MIPI_PORT2_ID

/* The CSI2 interface supports 2 types of compression or can
 *  be run without compression.
 */
/* Input port structure.
 */
struct ia_css_input_port {
	enum mipi_port_id port; /** Physical CSI-2 port */
	unsigned int num_lanes; /** Number of lanes used (4-lane port only) */
	unsigned int timeout;   /** Timeout value */
	unsigned int rxcount;   /** Register value, should include all lanes */
	struct ia_css_csi2_compression compression; /** Compression used */