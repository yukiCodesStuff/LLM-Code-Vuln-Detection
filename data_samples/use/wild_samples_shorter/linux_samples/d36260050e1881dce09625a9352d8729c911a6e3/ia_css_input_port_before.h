 * more details.
 */

#ifndef __IA_CSS_INPUT_PORT_H
#define __IA_CSS_INPUT_PORT_H

/* @file
 * This file contains information about the possible input ports for CSS
 */

/* Enumeration of the physical input ports on the CSS hardware.
 *  There are 3 MIPI CSI-2 ports.
 */
enum ia_css_csi2_port {
	IA_CSS_CSI2_PORT0, /* Implicitly map to MIPI_PORT0_ID */
	IA_CSS_CSI2_PORT1, /* Implicitly map to MIPI_PORT1_ID */
	IA_CSS_CSI2_PORT2  /* Implicitly map to MIPI_PORT2_ID */
};

/* Backward compatible for CSS API 2.0 only
 *  TO BE REMOVED when all drivers move to CSS	API 2.1
 */
#define	IA_CSS_CSI2_PORT_4LANE IA_CSS_CSI2_PORT0
#define	IA_CSS_CSI2_PORT_1LANE IA_CSS_CSI2_PORT1
#define	IA_CSS_CSI2_PORT_2LANE IA_CSS_CSI2_PORT2

/* The CSI2 interface supports 2 types of compression or can
 *  be run without compression.
 */
/* Input port structure.
 */
struct ia_css_input_port {
	enum ia_css_csi2_port port; /** Physical CSI-2 port */
	unsigned int num_lanes; /** Number of lanes used (4-lane port only) */
	unsigned int timeout;   /** Timeout value */
	unsigned int rxcount;   /** Register value, should include all lanes */
	struct ia_css_csi2_compression compression; /** Compression used */