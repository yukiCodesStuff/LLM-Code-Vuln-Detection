/*
 * Support for Intel Camera Imaging ISP subsystem.
 * Copyright (c) 2015, Intel Corporation.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 */

#include "gdc_device.h"		/* gdc_lut_store(), ... */
#include "isp.h"			/* ISP_VEC_ELEMBITS */
#include "vamem.h"
#if !defined(HAS_NO_HMEM)
#ifndef __INLINE_HMEM__
#define __INLINE_HMEM__
#endif
#include "hmem.h"
#endif /* !defined(HAS_NO_HMEM) */
#define IA_CSS_INCLUDE_PARAMETERS
#define IA_CSS_INCLUDE_ACC_PARAMETERS

#include "sh_css_params.h"
#include "ia_css_queue.h"
#include "sw_event_global.h"		/* Event IDs */

#include "platform_support.h"
#include "assert_support.h"
#include "misc_support.h"	/* NOT_USED */
#include "math_support.h"	/* max(), min()  EVEN_FLOOR()*/

#include "ia_css_stream.h"
#include "sh_css_params_internal.h"
#include "sh_css_param_shading.h"
#include "sh_css_param_dvs.h"
#include "ia_css_refcount.h"
#include "sh_css_internal.h"
#include "ia_css_control.h"
#include "ia_css_shading.h"
#include "sh_css_defs.h"
#include "sh_css_sp.h"
#include "ia_css_pipeline.h"
#include "ia_css_debug.h"
#include "memory_access.h"
#if 0   /* FIXME */
#include "memory_realloc.h"
#endif
#include "ia_css_isp_param.h"
#include "ia_css_isp_params.h"
#include "ia_css_mipi.h"
#include "ia_css_morph.h"
#include "ia_css_host_data.h"
#include "ia_css_pipe.h"
#include "ia_css_pipe_binarydesc.h"
#if 0
#include "ia_css_system_ctrl.h"
#endif

/* Include all kernel host interfaces for ISP1 */

#include "anr/anr_1.0/ia_css_anr.host.h"
#include "cnr/cnr_1.0/ia_css_cnr.host.h"
#include "csc/csc_1.0/ia_css_csc.host.h"
#include "de/de_1.0/ia_css_de.host.h"
#include "dp/dp_1.0/ia_css_dp.host.h"
#include "bnr/bnr_1.0/ia_css_bnr.host.h"
#include "dvs/dvs_1.0/ia_css_dvs.host.h"
#include "fpn/fpn_1.0/ia_css_fpn.host.h"
#include "gc/gc_1.0/ia_css_gc.host.h"
#include "macc/macc_1.0/ia_css_macc.host.h"
#include "ctc/ctc_1.0/ia_css_ctc.host.h"
#include "ob/ob_1.0/ia_css_ob.host.h"
#include "raw/raw_1.0/ia_css_raw.host.h"
#include "fixedbds/fixedbds_1.0/ia_css_fixedbds_param.h"
#include "s3a/s3a_1.0/ia_css_s3a.host.h"
#include "sc/sc_1.0/ia_css_sc.host.h"
#include "sdis/sdis_1.0/ia_css_sdis.host.h"
#include "tnr/tnr_1.0/ia_css_tnr.host.h"
#include "uds/uds_1.0/ia_css_uds_param.h"
#include "wb/wb_1.0/ia_css_wb.host.h"
#include "ynr/ynr_1.0/ia_css_ynr.host.h"
#include "xnr/xnr_1.0/ia_css_xnr.host.h"

/* Include additional kernel host interfaces for ISP2 */

#include "aa/aa_2/ia_css_aa2.host.h"
#include "anr/anr_2/ia_css_anr2.host.h"
#include "bh/bh_2/ia_css_bh.host.h"
#include "cnr/cnr_2/ia_css_cnr2.host.h"
#include "ctc/ctc1_5/ia_css_ctc1_5.host.h"
#include "de/de_2/ia_css_de2.host.h"
#include "gc/gc_2/ia_css_gc2.host.h"
#include "sdis/sdis_2/ia_css_sdis2.host.h"
#include "ynr/ynr_2/ia_css_ynr2.host.h"
#include "fc/fc_1.0/ia_css_formats.host.h"

#include "xnr/xnr_3.0/ia_css_xnr3.host.h"

#if defined(HAS_OUTPUT_SYSTEM)
#include <components/output_system/sc_output_system_1.0/host/output_system.host.h>
#endif

#include "sh_css_frac.h"
#include "ia_css_bufq.h"

#define FPNTBL_BYTES(binary) \
	(sizeof(char) * (binary)->in_frame_info.res.height * \
	 (binary)->in_frame_info.padded_width)
	 
#ifndef ISP2401

#define SCTBL_BYTES(binary) \
	(sizeof(unsigned short) * (binary)->sctbl_height * \
	 (binary)->sctbl_aligned_width_per_color * IA_CSS_SC_NUM_COLORS)

#else

#define SCTBL_BYTES(binary) \
	(sizeof(unsigned short) * max((binary)->sctbl_height, (binary)->sctbl_legacy_height) * \
			/* height should be the larger height between new api and legacy api */ \
	 (binary)->sctbl_aligned_width_per_color * IA_CSS_SC_NUM_COLORS)

#endif

#define MORPH_PLANE_BYTES(binary) \
	(SH_CSS_MORPH_TABLE_ELEM_BYTES * (binary)->morph_tbl_aligned_width * \
	 (binary)->morph_tbl_height)

/* We keep a second copy of the ptr struct for the SP to access.
   Again, this would not be necessary on the chip. */
static hrt_vaddress sp_ddr_ptrs;

/* sp group address on DDR */
static hrt_vaddress xmem_sp_group_ptrs;

static hrt_vaddress xmem_sp_stage_ptrs[IA_CSS_PIPE_ID_NUM]
						[SH_CSS_MAX_STAGES];
static hrt_vaddress xmem_isp_stage_ptrs[IA_CSS_PIPE_ID_NUM]
						[SH_CSS_MAX_STAGES];

static hrt_vaddress default_gdc_lut;
static int interleaved_lut_temp[4][HRT_GDC_N];

/* END DO NOT MOVE INTO VIMALS_WORLD */

/* Digital Zoom lookup table. See documentation for more details about the
 * contents of this table.
 */
#if defined(HAS_GDC_VERSION_2)
#if defined(CONFIG_CSI2_PLUS)
/*
 * Coefficients from
 * Css_Mizuchi/regressions/20140424_0930/all/applications/common/gdc_v2_common/lut.h
 */

static const int zoom_table[4][HRT_GDC_N] = {
	{	   0,    0,    0,    0,    0,    0,    0,    0,
		   0,    0,    0,    0,    0,    0,    0,    0,
		   0,    0,    0,    0,    0,    0,    0,   -1,
		  -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
		  -1,   -2,   -2,   -2,   -2,   -2,   -2,   -2,
		  -3,   -3,   -3,   -3,   -3,   -3,   -3,   -4,
		  -4,   -4,   -4,   -4,   -5,   -5,   -5,   -5,
		  -5,   -5,   -6,   -6,   -6,   -6,   -7,   -7,
		  -7,   -7,   -7,   -8,   -8,   -8,   -8,   -9,
		  -9,   -9,   -9,  -10,  -10,  -10,  -10,  -11,
		 -11,  -11,  -12,  -12,  -12,  -12,  -13,  -13,
		 -13,  -14,  -14,  -14,  -15,  -15,  -15,  -15,
		 -16,  -16,  -16,  -17,  -17,  -17,  -18,  -18,
		 -18,  -19,  -19,  -20,  -20,  -20,  -21,  -21,
		 -21,  -22,  -22,  -22,  -23,  -23,  -24,  -24,
		 -24,  -25,  -25,  -25,  -26,  -26,  -27,  -27,
		 -28,  -28,  -28,  -29,  -29,  -30,  -30,  -30,
		 -31,  -31,  -32,  -32,  -33,  -33,  -33,  -34,
		 -34,  -35,  -35,  -36,  -36,  -37,  -37,  -37,
		 -38,  -38,  -39,  -39,  -40,  -40,  -41,  -41,
		 -42,  -42,  -43,  -43,  -44,  -44,  -45,  -45,
		 -46,  -46,  -47,  -47,  -48,  -48,  -49,  -49,
		 -50,  -50,  -51,  -51,  -52,  -52,  -53,  -53,
		 -54,  -54,  -55,  -55,  -56,  -56,  -57,  -57,
		 -58,  -59,  -59,  -60,  -60,  -61,  -61,  -62,
		 -62,  -63,  -63,  -64,  -65,  -65,  -66,  -66,
		 -67,  -67,  -68,  -69,  -69,  -70,  -70,  -71,
		 -71,  -72,  -73,  -73,  -74,  -74,  -75,  -75,
		 -76,  -77,  -77,  -78,  -78,  -79,  -80,  -80,
		 -81,  -81,  -82,  -83,  -83,  -84,  -84,  -85,
		 -86,  -86,  -87,  -87,  -88,  -89,  -89,  -90,
		 -91,  -91,  -92,  -92,  -93,  -94,  -94,  -95,
		 -96,  -96,  -97,  -97,  -98,  -99,  -99, -100,
		-101, -101, -102, -102, -103, -104, -104, -105,
		-106, -106, -107, -108, -108, -109, -109, -110,
		-111, -111, -112, -113, -113, -114, -115, -115,
		-116, -117, -117, -118, -119, -119, -120, -121,
		-121, -122, -122, -123, -124, -124, -125, -126,
		-126, -127, -128, -128, -129, -130, -130, -131,
		-132, -132, -133, -134, -134, -135, -136, -136,
		-137, -138, -138, -139, -140, -140, -141, -142,
		-142, -143, -144, -144, -145, -146, -146, -147,
		-148, -148, -149, -150, -150, -151, -152, -152,
		-153, -154, -154, -155, -156, -156, -157, -158,
		-158, -159, -160, -160, -161, -162, -162, -163,
		-164, -164, -165, -166, -166, -167, -168, -168,
		-169, -170, -170, -171, -172, -172, -173, -174,
		-174, -175, -176, -176, -177, -178, -178, -179,
		-180, -180, -181, -181, -182, -183, -183, -184,
		-185, -185, -186, -187, -187, -188, -189, -189,
		-190, -191, -191, -192, -193, -193, -194, -194,
		-195, -196, -196, -197, -198, -198, -199, -200,
		-200, -201, -201, -202, -203, -203, -204, -205,
		-205, -206, -206, -207, -208, -208, -209, -210,
		-210, -211, -211, -212, -213, -213, -214, -215,
		-215, -216, -216, -217, -218, -218, -219, -219,
		-220, -221, -221, -222, -222, -223, -224, -224,
		-225, -225, -226, -227, -227, -228, -228, -229,
		-229, -230, -231, -231, -232, -232, -233, -233,
		-234, -235, -235, -236, -236, -237, -237, -238,
		-239, -239, -240, -240, -241, -241, -242, -242,
		-243, -244, -244, -245, -245, -246, -246, -247,
		-247, -248, -248, -249, -249, -250, -250, -251,
		-251, -252, -252, -253, -253, -254, -254, -255,
		-256, -256, -256, -257, -257, -258, -258, -259,
		-259, -260, -260, -261, -261, -262, -262, -263,
		-263, -264, -264, -265, -265, -266, -266, -266,
		-267, -267, -268, -268, -269, -269, -270, -270,
		-270, -271, -271, -272, -272, -273, -273, -273,
		-274, -274, -275, -275, -275, -276, -276, -277,
		-277, -277, -278, -278, -279, -279, -279, -280,
		-280, -280, -281, -281, -282, -282, -282, -283,
		-283, -283, -284, -284, -284, -285, -285, -285,
		-286, -286, -286, -287, -287, -287, -288, -288,
		-288, -289, -289, -289, -289, -290, -290, -290,
		-291, -291, -291, -291, -292, -292, -292, -293,
		-293, -293, -293, -294, -294, -294, -294, -295,
		-295, -295, -295, -295, -296, -296, -296, -296,
		-297, -297, -297, -297, -297, -298, -298, -298,
		-298, -298, -299, -299, -299, -299, -299, -299,
		-300, -300, -300, -300, -300, -300, -300, -301,
		-301, -301, -301, -301, -301, -301, -301, -301,
		-302, -302, -302, -302, -302, -302, -302, -302,
		-302, -302, -302, -302, -302, -303, -303, -303,
		-303, -303, -303, -303, -303, -303, -303, -303,
		-303, -303, -303, -303, -303, -303, -303, -303,
		-303, -303, -303, -303, -303, -303, -303, -303,
		-303, -303, -302, -302, -302, -302, -302, -302,
		-302, -302, -302, -302, -302, -302, -301, -301,
		-301, -301, -301, -301, -301, -301, -300, -300,
		-300, -300, -300, -300, -299, -299, -299, -299,
		-299, -299, -298, -298, -298, -298, -298, -297,
		-297, -297, -297, -296, -296, -296, -296, -295,
		-295, -295, -295, -294, -294, -294, -293, -293,
		-293, -293, -292, -292, -292, -291, -291, -291,
		-290, -290, -290, -289, -289, -289, -288, -288,
		-288, -287, -287, -286, -286, -286, -285, -285,
		-284, -284, -284, -283, -283, -282, -282, -281,
		-281, -280, -280, -279, -279, -279, -278, -278,
		-277, -277, -276, -276, -275, -275, -274, -273,
		-273, -272, -272, -271, -271, -270, -270, -269,
		-268, -268, -267, -267, -266, -266, -265, -264,
		-264, -263, -262, -262, -261, -260, -260, -259,
		-259, -258, -257, -256, -256, -255, -254, -254,
		-253, -252, -252, -251, -250, -249, -249, -248,
		-247, -246, -246, -245, -244, -243, -242, -242,
		-241, -240, -239, -238, -238, -237, -236, -235,
		-234, -233, -233, -232, -231, -230, -229, -228,
		-227, -226, -226, -225, -224, -223, -222, -221,
		-220, -219, -218, -217, -216, -215, -214, -213,
		-212, -211, -210, -209, -208, -207, -206, -205,
		-204, -203, -202, -201, -200, -199, -198, -197,
		-196, -194, -193, -192, -191, -190, -189, -188,
		-187, -185, -184, -183, -182, -181, -180, -178,
		-177, -176, -175, -174, -172, -171, -170, -169,
		-167, -166, -165, -164, -162, -161, -160, -158,
		-157, -156, -155, -153, -152, -151, -149, -148,
		-147, -145, -144, -142, -141, -140, -138, -137,
		-135, -134, -133, -131, -130, -128, -127, -125,
		-124, -122, -121, -120, -118, -117, -115, -114,
		-112, -110, -109, -107, -106, -104, -103, -101,
		-100,  -98,  -96,  -95,  -93,  -92,  -90,  -88,
		 -87,  -85,  -83,  -82,  -80,  -78,  -77,  -75,
		 -73,  -72,  -70,  -68,  -67,  -65,  -63,  -61,
		 -60,  -58,  -56,  -54,  -52,  -51,  -49,  -47,
		 -45,  -43,  -42,  -40,  -38,  -36,  -34,  -32,
		 -31,  -29,  -27,  -25,  -23,  -21,  -19,  -17,
		 -15,  -13,  -11,   -9,   -7,   -5,   -3,   -1
	},
	{	   0,    2,    4,    6,    8,   10,   12,   14,
		  16,   18,   20,   22,   25,   27,   29,   31,
		  33,   36,   38,   40,   43,   45,   47,   50,
		  52,   54,   57,   59,   61,   64,   66,   69,
		  71,   74,   76,   79,   81,   84,   86,   89,
		  92,   94,   97,   99,  102,  105,  107,  110,
		 113,  116,  118,  121,  124,  127,  129,  132,
		 135,  138,  141,  144,  146,  149,  152,  155,
		 158,  161,  164,  167,  170,  173,  176,  179,
		 182,  185,  188,  191,  194,  197,  200,  203,
		 207,  210,  213,  216,  219,  222,  226,  229,
		 232,  235,  239,  242,  245,  248,  252,  255,
		 258,  262,  265,  269,  272,  275,  279,  282,
		 286,  289,  292,  296,  299,  303,  306,  310,
		 313,  317,  321,  324,  328,  331,  335,  338,
		 342,  346,  349,  353,  357,  360,  364,  368,
		 372,  375,  379,  383,  386,  390,  394,  398,
		 402,  405,  409,  413,  417,  421,  425,  429,
		 432,  436,  440,  444,  448,  452,  456,  460,
		 464,  468,  472,  476,  480,  484,  488,  492,
		 496,  500,  504,  508,  512,  516,  521,  525,
		 529,  533,  537,  541,  546,  550,  554,  558,
		 562,  567,  571,  575,  579,  584,  588,  592,
		 596,  601,  605,  609,  614,  618,  622,  627,
		 631,  635,  640,  644,  649,  653,  657,  662,
		 666,  671,  675,  680,  684,  689,  693,  698,
		 702,  707,  711,  716,  720,  725,  729,  734,
		 738,  743,  747,  752,  757,  761,  766,  771,
		 775,  780,  784,  789,  794,  798,  803,  808,
		 813,  817,  822,  827,  831,  836,  841,  846,
		 850,  855,  860,  865,  870,  874,  879,  884,
		 889,  894,  898,  903,  908,  913,  918,  923,
		 928,  932,  937,  942,  947,  952,  957,  962,
		 967,  972,  977,  982,  986,  991,  996, 1001,
		1006, 1011, 1016, 1021, 1026, 1031, 1036, 1041,
		1046, 1051, 1056, 1062, 1067, 1072, 1077, 1082,
		1087, 1092, 1097, 1102, 1107, 1112, 1117, 1122,
		1128, 1133, 1138, 1143, 1148, 1153, 1158, 1164,
		1169, 1174, 1179, 1184, 1189, 1195, 1200, 1205,
		1210, 1215, 1221, 1226, 1231, 1236, 1242, 1247,
		1252, 1257, 1262, 1268, 1273, 1278, 1284, 1289,
		1294, 1299, 1305, 1310, 1315, 1321, 1326, 1331,
		1336, 1342, 1347, 1352, 1358, 1363, 1368, 1374,
		1379, 1384, 1390, 1395, 1400, 1406, 1411, 1417,
		1422, 1427, 1433, 1438, 1443, 1449, 1454, 1460,
		1465, 1470, 1476, 1481, 1487, 1492, 1497, 1503,
		1508, 1514, 1519, 1525, 1530, 1535, 1541, 1546,
		1552, 1557, 1563, 1568, 1574, 1579, 1585, 1590,
		1596, 1601, 1606, 1612, 1617, 1623, 1628, 1634,
		1639, 1645, 1650, 1656, 1661, 1667, 1672, 1678,
		1683, 1689, 1694, 1700, 1705, 1711, 1716, 1722,
		1727, 1733, 1738, 1744, 1749, 1755, 1761, 1766,
		1772, 1777, 1783, 1788, 1794, 1799, 1805, 1810,
		1816, 1821, 1827, 1832, 1838, 1844, 1849, 1855,
		1860, 1866, 1871, 1877, 1882, 1888, 1893, 1899,
		1905, 1910, 1916, 1921, 1927, 1932, 1938, 1943,
		1949, 1955, 1960, 1966, 1971, 1977, 1982, 1988,
		1993, 1999, 2005, 2010, 2016, 2021, 2027, 2032,
		2038, 2043, 2049, 2055, 2060, 2066, 2071, 2077,
		2082, 2088, 2093, 2099, 2105, 2110, 2116, 2121,
		2127, 2132, 2138, 2143, 2149, 2154, 2160, 2165,
		2171, 2177, 2182, 2188, 2193, 2199, 2204, 2210,
		2215, 2221, 2226, 2232, 2237, 2243, 2248, 2254,
		2259, 2265, 2270, 2276, 2281, 2287, 2292, 2298,
		2304, 2309, 2314, 2320, 2325, 2331, 2336, 2342,
		2347, 2353, 2358, 2364, 2369, 2375, 2380, 2386,
		2391, 2397, 2402, 2408, 2413, 2419, 2424, 2429,
		2435, 2440, 2446, 2451, 2457, 2462, 2467, 2473,
		2478, 2484, 2489, 2495, 2500, 2505, 2511, 2516,
		2522, 2527, 2532, 2538, 2543, 2549, 2554, 2559,
		2565, 2570, 2575, 2581, 2586, 2591, 2597, 2602,
		2607, 2613, 2618, 2623, 2629, 2634, 2639, 2645,
		2650, 2655, 2661, 2666, 2671, 2676, 2682, 2687,
		2692, 2698, 2703, 2708, 2713, 2719, 2724, 2729,
		2734, 2740, 2745, 2750, 2755, 2760, 2766, 2771,
		2776, 2781, 2786, 2792, 2797, 2802, 2807, 2812,
		2817, 2823, 2828, 2833, 2838, 2843, 2848, 2853,
		2859, 2864, 2869, 2874, 2879, 2884, 2889, 2894,
		2899, 2904, 2909, 2914, 2919, 2924, 2930, 2935,
		2940, 2945, 2950, 2955, 2960, 2965, 2970, 2975,
		2980, 2984, 2989, 2994, 2999, 3004, 3009, 3014,
		3019, 3024, 3029, 3034, 3039, 3044, 3048, 3053,
		3058, 3063, 3068, 3073, 3078, 3082, 3087, 3092,
		3097, 3102, 3106, 3111, 3116, 3121, 3126, 3130,
		3135, 3140, 3145, 3149, 3154, 3159, 3163, 3168,
		3173, 3177, 3182, 3187, 3191, 3196, 3201, 3205,
		3210, 3215, 3219, 3224, 3228, 3233, 3238, 3242,
		3247, 3251, 3256, 3260, 3265, 3269, 3274, 3279,
		3283, 3287, 3292, 3296, 3301, 3305, 3310, 3314,
		3319, 3323, 3327, 3332, 3336, 3341, 3345, 3349,
		3354, 3358, 3362, 3367, 3371, 3375, 3380, 3384,
		3388, 3393, 3397, 3401, 3405, 3410, 3414, 3418,
		3422, 3426, 3431, 3435, 3439, 3443, 3447, 3451,
		3455, 3460, 3464, 3468, 3472, 3476, 3480, 3484,
		3488, 3492, 3496, 3500, 3504, 3508, 3512, 3516,
		3520, 3524, 3528, 3532, 3536, 3540, 3544, 3548,
		3552, 3555, 3559, 3563, 3567, 3571, 3575, 3578,
		3582, 3586, 3590, 3593, 3597, 3601, 3605, 3608,
		3612, 3616, 3619, 3623, 3627, 3630, 3634, 3638,
		3641, 3645, 3649, 3652, 3656, 3659, 3663, 3666,
		3670, 3673, 3677, 3680, 3684, 3687, 3691, 3694,
		3698, 3701, 3704, 3708, 3711, 3714, 3718, 3721,
		3724, 3728, 3731, 3734, 3738, 3741, 3744, 3747,
		3751, 3754, 3757, 3760, 3763, 3767, 3770, 3773,
		3776, 3779, 3782, 3785, 3788, 3791, 3794, 3798,
		3801, 3804, 3807, 3809, 3812, 3815, 3818, 3821,
		3824, 3827, 3830, 3833, 3836, 3839, 3841, 3844,
		3847, 3850, 3853, 3855, 3858, 3861, 3864, 3866,
		3869, 3872, 3874, 3877, 3880, 3882, 3885, 3887,
		3890, 3893, 3895, 3898, 3900, 3903, 3905, 3908,
		3910, 3913, 3915, 3917, 3920, 3922, 3925, 3927,
		3929, 3932, 3934, 3936, 3939, 3941, 3943, 3945,
		3948, 3950, 3952, 3954, 3956, 3958, 3961, 3963,
		3965, 3967, 3969, 3971, 3973, 3975, 3977, 3979,
		3981, 3983, 3985, 3987, 3989, 3991, 3993, 3994,
		3996, 3998, 4000, 4002, 4004, 4005, 4007, 4009,
		4011, 4012, 4014, 4016, 4017, 4019, 4021, 4022,
		4024, 4025, 4027, 4028, 4030, 4031, 4033, 4034,
		4036, 4037, 4039, 4040, 4042, 4043, 4044, 4046,
		4047, 4048, 4050, 4051, 4052, 4053, 4055, 4056,
		4057, 4058, 4059, 4060, 4062, 4063, 4064, 4065,
		4066, 4067, 4068, 4069, 4070, 4071, 4072, 4073,
		4074, 4075, 4075, 4076, 4077, 4078, 4079, 4079,
		4080, 4081, 4082, 4082, 4083, 4084, 4084, 4085,
		4086, 4086, 4087, 4087, 4088, 4088, 4089, 4089,
		4090, 4090, 4091, 4091, 4092, 4092, 4092, 4093,
		4093, 4093, 4094, 4094, 4094, 4094, 4095, 4095,
		4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095
	},
	{	4096, 4095, 4095, 4095, 4095, 4095, 4095, 4095,
		4095, 4095, 4095, 4094, 4094, 4094, 4094, 4093,
		4093, 4093, 4092, 4092, 4092, 4091, 4091, 4090,
		4090, 4089, 4089, 4088, 4088, 4087, 4087, 4086,
		4086, 4085, 4084, 4084, 4083, 4082, 4082, 4081,
		4080, 4079, 4079, 4078, 4077, 4076, 4075, 4075,
		4074, 4073, 4072, 4071, 4070, 4069, 4068, 4067,
		4066, 4065, 4064, 4063, 4062, 4060, 4059, 4058,
		4057, 4056, 4055, 4053, 4052, 4051, 4050, 4048,
		4047, 4046, 4044, 4043, 4042, 4040, 4039, 4037,
		4036, 4034, 4033, 4031, 4030, 4028, 4027, 4025,
		4024, 4022, 4021, 4019, 4017, 4016, 4014, 4012,
		4011, 4009, 4007, 4005, 4004, 4002, 4000, 3998,
		3996, 3994, 3993, 3991, 3989, 3987, 3985, 3983,
		3981, 3979, 3977, 3975, 3973, 3971, 3969, 3967,
		3965, 3963, 3961, 3958, 3956, 3954, 3952, 3950,
		3948, 3945, 3943, 3941, 3939, 3936, 3934, 3932,
		3929, 3927, 3925, 3922, 3920, 3917, 3915, 3913,
		3910, 3908, 3905, 3903, 3900, 3898, 3895, 3893,
		3890, 3887, 3885, 3882, 3880, 3877, 3874, 3872,
		3869, 3866, 3864, 3861, 3858, 3855, 3853, 3850,
		3847, 3844, 3841, 3839, 3836, 3833, 3830, 3827,
		3824, 3821, 3818, 3815, 3812, 3809, 3807, 3804,
		3801, 3798, 3794, 3791, 3788, 3785, 3782, 3779,
		3776, 3773, 3770, 3767, 3763, 3760, 3757, 3754,
		3751, 3747, 3744, 3741, 3738, 3734, 3731, 3728,
		3724, 3721, 3718, 3714, 3711, 3708, 3704, 3701,
		3698, 3694, 3691, 3687, 3684, 3680, 3677, 3673,
		3670, 3666, 3663, 3659, 3656, 3652, 3649, 3645,
		3641, 3638, 3634, 3630, 3627, 3623, 3619, 3616,
		3612, 3608, 3605, 3601, 3597, 3593, 3590, 3586,
		3582, 3578, 3575, 3571, 3567, 3563, 3559, 3555,
		3552, 3548, 3544, 3540, 3536, 3532, 3528, 3524,
		3520, 3516, 3512, 3508, 3504, 3500, 3496, 3492,
		3488, 3484, 3480, 3476, 3472, 3468, 3464, 3460,
		3455, 3451, 3447, 3443, 3439, 3435, 3431, 3426,
		3422, 3418, 3414, 3410, 3405, 3401, 3397, 3393,
		3388, 3384, 3380, 3375, 3371, 3367, 3362, 3358,
		3354, 3349, 3345, 3341, 3336, 3332, 3327, 3323,
		3319, 3314, 3310, 3305, 3301, 3296, 3292, 3287,
		3283, 3279, 3274, 3269, 3265, 3260, 3256, 3251,
		3247, 3242, 3238, 3233, 3228, 3224, 3219, 3215,
		3210, 3205, 3201, 3196, 3191, 3187, 3182, 3177,
		3173, 3168, 3163, 3159, 3154, 3149, 3145, 3140,
		3135, 3130, 3126, 3121, 3116, 3111, 3106, 3102,
		3097, 3092, 3087, 3082, 3078, 3073, 3068, 3063,
		3058, 3053, 3048, 3044, 3039, 3034, 3029, 3024,
		3019, 3014, 3009, 3004, 2999, 2994, 2989, 2984,
		2980, 2975, 2970, 2965, 2960, 2955, 2950, 2945,
		2940, 2935, 2930, 2924, 2919, 2914, 2909, 2904,
		2899, 2894, 2889, 2884, 2879, 2874, 2869, 2864,
		2859, 2853, 2848, 2843, 2838, 2833, 2828, 2823,
		2817, 2812, 2807, 2802, 2797, 2792, 2786, 2781,
		2776, 2771, 2766, 2760, 2755, 2750, 2745, 2740,
		2734, 2729, 2724, 2719, 2713, 2708, 2703, 2698,
		2692, 2687, 2682, 2676, 2671, 2666, 2661, 2655,
		2650, 2645, 2639, 2634, 2629, 2623, 2618, 2613,
		2607, 2602, 2597, 2591, 2586, 2581, 2575, 2570,
		2565, 2559, 2554, 2549, 2543, 2538, 2532, 2527,
		2522, 2516, 2511, 2505, 2500, 2495, 2489, 2484,
		2478, 2473, 2467, 2462, 2457, 2451, 2446, 2440,
		2435, 2429, 2424, 2419, 2413, 2408, 2402, 2397,
		2391, 2386, 2380, 2375, 2369, 2364, 2358, 2353,
		2347, 2342, 2336, 2331, 2325, 2320, 2314, 2309,
		2304, 2298, 2292, 2287, 2281, 2276, 2270, 2265,
		2259, 2254, 2248, 2243, 2237, 2232, 2226, 2221,
		2215, 2210, 2204, 2199, 2193, 2188, 2182, 2177,
		2171, 2165, 2160, 2154, 2149, 2143, 2138, 2132,
		2127, 2121, 2116, 2110, 2105, 2099, 2093, 2088,
		2082, 2077, 2071, 2066, 2060, 2055, 2049, 2043,
		2038, 2032, 2027, 2021, 2016, 2010, 2005, 1999,
		1993, 1988, 1982, 1977, 1971, 1966, 1960, 1955,
		1949, 1943, 1938, 1932, 1927, 1921, 1916, 1910,
		1905, 1899, 1893, 1888, 1882, 1877, 1871, 1866,
		1860, 1855, 1849, 1844, 1838, 1832, 1827, 1821,
		1816, 1810, 1805, 1799, 1794, 1788, 1783, 1777,
		1772, 1766, 1761, 1755, 1749, 1744, 1738, 1733,
		1727, 1722, 1716, 1711, 1705, 1700, 1694, 1689,
		1683, 1678, 1672, 1667, 1661, 1656, 1650, 1645,
		1639, 1634, 1628, 1623, 1617, 1612, 1606, 1601,
		1596, 1590, 1585, 1579, 1574, 1568, 1563, 1557,
		1552, 1546, 1541, 1535, 1530, 1525, 1519, 1514,
		1508, 1503, 1497, 1492, 1487, 1481, 1476, 1470,
		1465, 1460, 1454, 1449, 1443, 1438, 1433, 1427,
		1422, 1417, 1411, 1406, 1400, 1395, 1390, 1384,
		1379, 1374, 1368, 1363, 1358, 1352, 1347, 1342,
		1336, 1331, 1326, 1321, 1315, 1310, 1305, 1299,
		1294, 1289, 1284, 1278, 1273, 1268, 1262, 1257,
		1252, 1247, 1242, 1236, 1231, 1226, 1221, 1215,
		1210, 1205, 1200, 1195, 1189, 1184, 1179, 1174,
		1169, 1164, 1158, 1153, 1148, 1143, 1138, 1133,
		1128, 1122, 1117, 1112, 1107, 1102, 1097, 1092,
		1087, 1082, 1077, 1072, 1067, 1062, 1056, 1051,
		1046, 1041, 1036, 1031, 1026, 1021, 1016, 1011,
		1006, 1001,  996,  991,  986,  982,  977,  972,
		 967,  962,  957,  952,  947,  942,  937,  932,
		 928,  923,  918,  913,  908,  903,  898,  894,
		 889,  884,  879,  874,  870,  865,  860,  855,
		 850,  846,  841,  836,  831,  827,  822,  817,
		 813,  808,  803,  798,  794,  789,  784,  780,
		 775,  771,  766,  761,  757,  752,  747,  743,
		 738,  734,  729,  725,  720,  716,  711,  707,
		 702,  698,  693,  689,  684,  680,  675,  671,
		 666,  662,  657,  653,  649,  644,  640,  635,
		 631,  627,  622,  618,  614,  609,  605,  601,
		 596,  592,  588,  584,  579,  575,  571,  567,
		 562,  558,  554,  550,  546,  541,  537,  533,
		 529,  525,  521,  516,  512,  508,  504,  500,
		 496,  492,  488,  484,  480,  476,  472,  468,
		 464,  460,  456,  452,  448,  444,  440,  436,
		 432,  429,  425,  421,  417,  413,  409,  405,
		 402,  398,  394,  390,  386,  383,  379,  375,
		 372,  368,  364,  360,  357,  353,  349,  346,
		 342,  338,  335,  331,  328,  324,  321,  317,
		 313,  310,  306,  303,  299,  296,  292,  289,
		 286,  282,  279,  275,  272,  269,  265,  262,
		 258,  255,  252,  248,  245,  242,  239,  235,
		 232,  229,  226,  222,  219,  216,  213,  210,
		 207,  203,  200,  197,  194,  191,  188,  185,
		 182,  179,  176,  173,  170,  167,  164,  161,
		 158,  155,  152,  149,  146,  144,  141,  138,
		 135,  132,  129,  127,  124,  121,  118,  116,
		 113,  110,  107,  105,  102,   99,   97,   94,
		  92,   89,   86,   84,   81,   79,   76,   74,
		  71,   69,   66,   64,   61,   59,   57,   54,
		  52,   50,   47,   45,   43,   40,   38,   36,
		  33,   31,   29,   27,   25,   22,   20,   18,
		  16,   14,   12,   10,    8,    6,    4,    2
	},
	{	   0,   -1,   -3,   -5,   -7,   -9,  -11,  -13,
		 -15,  -17,  -19,  -20,  -23,  -25,  -27,  -28,
		 -30,  -33,  -34,  -36,  -39,  -40,  -42,  -43,
		 -45,  -46,  -49,  -50,  -52,  -54,  -56,  -58,
		 -60,  -61,  -62,  -65,  -66,  -68,  -70,  -72,
		 -73,  -74,  -77,  -78,  -80,  -82,  -83,  -85,
		 -87,  -89,  -90,  -92,  -93,  -95,  -96,  -98,
		-100, -102, -103, -105, -106, -107, -108, -110,
		-112, -114, -116, -116, -118, -120, -122, -122,
		-124, -126, -127, -128, -130, -131, -133, -133,
		-136, -137, -138, -139, -141, -142, -144, -145,
		-147, -147, -150, -151, -151, -153, -155, -156,
		-157, -159, -160, -161, -163, -164, -165, -166,
		-168, -168, -170, -171, -172, -174, -174, -176,
		-177, -178, -180, -181, -182, -183, -184, -185,
		-187, -188, -189, -190, -191, -192, -193, -195,
		-196, -196, -198, -199, -200, -200, -202, -204,
		-204, -205, -206, -207, -208, -209, -211, -212,
		-212, -213, -214, -215, -216, -217, -218, -220,
		-220, -221, -222, -223, -224, -225, -225, -227,
		-227, -228, -229, -230, -230, -231, -233, -234,
		-234, -235, -235, -237, -238, -239, -239, -240,
		-240, -242, -242, -243, -243, -245, -246, -247,
		-247, -249, -248, -249, -250, -251, -251, -253,
		-253, -253, -255, -255, -256, -256, -257, -258,
		-259, -259, -260, -261, -261, -262, -262, -264,
		-263, -265, -265, -265, -266, -267, -267, -268,
		-269, -269, -269, -270, -271, -271, -272, -273,
		-273, -273, -274, -274, -276, -275, -276, -277,
		-277, -278, -278, -278, -279, -279, -280, -281,
		-280, -281, -282, -283, -283, -282, -284, -284,
		-284, -285, -285, -286, -286, -286, -287, -287,
		-288, -288, -288, -289, -289, -289, -290, -290,
		-290, -291, -291, -292, -291, -291, -292, -292,
		-292, -293, -293, -293, -294, -294, -295, -295,
		-294, -295, -295, -296, -297, -297, -297, -297,
		-297, -297, -298, -298, -297, -298, -298, -298,
		-299, -299, -300, -299, -299, -300, -299, -300,
		-301, -300, -300, -301, -300, -301, -301, -301,
		-301, -301, -302, -301, -302, -301, -302, -302,
		-302, -302, -302, -302, -302, -302, -303, -302,
		-303, -302, -303, -303, -302, -303, -303, -303,
		-302, -303, -303, -302, -303, -303, -302, -303,
		-303, -302, -303, -303, -302, -303, -303, -303,
		-303, -302, -303, -303, -302, -302, -302, -303,
		-302, -302, -302, -301, -303, -302, -301, -302,
		-301, -301, -301, -302, -301, -301, -301, -300,
		-301, -300, -300, -300, -300, -299, -300, -299,
		-300, -300, -299, -300, -299, -299, -299, -299,
		-298, -299, -298, -297, -297, -297, -296, -297,
		-296, -296, -296, -296, -295, -296, -295, -296,
		-295, -294, -294, -294, -293, -294, -294, -293,
		-293, -292, -293, -292, -292, -292, -291, -290,
		-291, -290, -291, -289, -289, -290, -289, -289,
		-288, -288, -288, -288, -286, -287, -286, -286,
		-286, -285, -286, -284, -284, -284, -284, -283,
		-283, -283, -282, -282, -282, -281, -280, -281,
		-279, -280, -280, -278, -279, -278, -278, -277,
		-278, -276, -276, -277, -275, -276, -274, -275,
		-274, -273, -273, -272, -273, -272, -272, -271,
		-270, -270, -269, -269, -269, -268, -268, -267,
		-267, -266, -266, -266, -265, -265, -264, -264,
		-263, -263, -262, -262, -261, -261, -260, -260,
		-259, -259, -258, -258, -257, -257, -256, -256,
		-256, -255, -254, -254, -253, -253, -252, -252,
		-251, -251, -250, -250, -249, -249, -248, -248,
		-247, -247, -246, -246, -245, -245, -244, -244,
		-243, -242, -242, -241, -241, -240, -239, -239,
		-239, -238, -238, -237, -237, -235, -235, -235,
		-234, -234, -232, -233, -232, -232, -231, -229,
		-230, -229, -228, -228, -227, -226, -227, -225,
		-224, -225, -223, -223, -222, -222, -221, -221,
		-220, -219, -219, -218, -218, -216, -217, -216,
		-215, -215, -214, -213, -212, -213, -211, -211,
		-210, -210, -209, -209, -208, -206, -207, -206,
		-205, -204, -204, -204, -203, -202, -202, -200,
		-200, -200, -200, -198, -197, -197, -196, -195,
		-195, -195, -194, -194, -192, -192, -191, -191,
		-189, -189, -188, -188, -187, -186, -186, -186,
		-185, -185, -183, -183, -182, -182, -181, -181,
		-180, -178, -178, -177, -177, -176, -176, -174,
		-174, -173, -173, -172, -172, -172, -170, -170,
		-168, -168, -167, -167, -167, -165, -165, -164,
		-164, -164, -162, -162, -161, -160, -160, -158,
		-158, -158, -157, -156, -155, -155, -154, -153,
		-153, -152, -151, -151, -150, -149, -149, -148,
		-147, -147, -146, -146, -144, -144, -144, -142,
		-142, -141, -142, -140, -140, -139, -138, -138,
		-137, -136, -136, -134, -134, -133, -134, -132,
		-132, -131, -130, -130, -128, -128, -128, -127,
		-127, -126, -124, -124, -124, -123, -123, -122,
		-121, -120, -120, -119, -118, -118, -117, -117,
		-116, -115, -115, -115, -114, -113, -111, -111,
		-110, -110, -109, -109, -108, -107, -107, -106,
		-105, -104, -104, -103, -102, -103, -102, -101,
		-101, -100,  -99,  -99,  -98,  -97,  -97,  -96,
		 -96,  -95,  -94,  -94,  -93,  -92,  -92,  -91,
		 -91,  -90,  -89,  -88,  -88,  -88,  -87,  -86,
		 -85,  -86,  -84,  -84,  -83,  -82,  -82,  -81,
		 -81,  -80,  -80,  -78,  -79,  -77,  -77,  -77,
		 -76,  -76,  -75,  -74,  -74,  -73,  -72,  -72,
		 -72,  -71,  -70,  -70,  -69,  -68,  -68,  -68,
		 -66,  -67,  -66,  -65,  -65,  -65,  -63,  -63,
		 -62,  -62,  -61,  -61,  -60,  -60,  -60,  -58,
		 -58,  -58,  -56,  -56,  -56,  -55,  -54,  -55,
		 -54,  -54,  -53,  -52,  -51,  -51,  -51,  -50,
		 -49,  -49,  -49,  -49,  -48,  -47,  -46,  -46,
		 -46,  -46,  -45,  -43,  -43,  -43,  -43,  -42,
		 -42,  -42,  -40,  -40,  -40,  -39,  -39,  -38,
		 -38,  -38,  -37,  -37,  -36,  -36,  -35,  -35,
		 -34,  -35,  -34,  -33,  -33,  -32,  -32,  -31,
		 -31,  -31,  -30,  -29,  -29,  -29,  -28,  -27,
		 -28,  -28,  -27,  -26,  -26,  -25,  -25,  -25,
		 -24,  -24,  -24,  -23,  -23,  -22,  -22,  -22,
		 -21,  -21,  -20,  -20,  -20,  -20,  -19,  -18,
		 -19,  -18,  -18,  -17,  -18,  -17,  -16,  -17,
		 -16,  -15,  -15,  -15,  -14,  -14,  -15,  -13,
		 -13,  -13,  -13,  -12,  -12,  -11,  -12,  -11,
		 -12,  -10,  -10,  -10,  -10,  -10,   -9,  -10,
		  -9,   -9,   -9,   -8,   -8,   -7,   -8,   -7,
		  -7,   -7,   -6,   -6,   -6,   -7,   -6,   -6,
		  -5,   -5,   -5,   -5,   -5,   -4,   -4,   -5,
		  -4,   -4,   -3,   -3,   -3,   -3,   -3,   -2,
		  -3,   -2,   -2,   -2,   -1,   -2,   -1,   -2,
		  -1,   -1,   -1,   -1,   -1,    0,   -1,    0,
		  -1,   -1,    0,    0,   -1,    0,    0,   -1,
		   1,    1,    0,    0,    0,    1,    0,    0,
		   0,    0,    0,    0,    0,    0,    0,    0
	}
		} else {
			const struct sh_css_binary_args *args = &stage->args;
			const struct ia_css_frame_info *out_infos[IA_CSS_BINARY_MAX_OUTPUT_PORTS] = {NULL};
			if (args->out_frame[0])
				out_infos[0] = &args->out_frame[0]->info;
			info = &stage->firmware->info.isp;
			ia_css_binary_fill_info(info, false, false,
				IA_CSS_STREAM_FORMAT_RAW_10,
				args->in_frame  ? &args->in_frame->info  : NULL,
				NULL,
				out_infos,
				args->out_vf_frame ? &args->out_vf_frame->info
									: NULL,
				&tmp_binary,
				NULL,
				-1, true);
			binary = &tmp_binary;
			binary->info = info;
		}

		if (stage == first_stage) {
			/* we will use pipe_in_res to scale the zoom crop region if needed */
			pipe_in_res = binary->effective_in_frame_res;
		}

		assert(stage->stage_num < SH_CSS_MAX_STAGES);
		if (params->dz_config.zoom_region.resolution.width == 0 &&
		    params->dz_config.zoom_region.resolution.height == 0) {
			sh_css_update_uds_and_crop_info(
				&info->sp,
				&binary->in_frame_info,
				&binary->out_frame_info[0],
				&binary->dvs_envelope,
				&params->dz_config,
				&params->motion_config,
				&params->uds[stage->stage_num].uds,
				&params->uds[stage->stage_num].crop_pos,
				stage->enable_zoom);
		} else {
			err = sh_css_update_uds_and_crop_info_based_on_zoom_region(
				&info->sp,
				&binary->in_frame_info,
				&binary->out_frame_info[0],
				&binary->dvs_envelope,
				&params->dz_config,
				&params->motion_config,
				&params->uds[stage->stage_num].uds,
				&params->uds[stage->stage_num].crop_pos,
				pipe_in_res,
				stage->enable_zoom);
			if (err != IA_CSS_SUCCESS)
			    return err;
		}
	if (bufs != NULL) {
		for (i = 0; i < num_bufs; i++)
			ia_css_metadata_free(bufs[i]);
	}
}

unsigned g_param_buffer_dequeue_count = 0;
unsigned g_param_buffer_enqueue_count = 0;

enum ia_css_err
ia_css_stream_isp_parameters_init(struct ia_css_stream *stream)
{
	enum ia_css_err err = IA_CSS_SUCCESS;
	unsigned i;
	struct sh_css_ddr_address_map *ddr_ptrs;
	struct sh_css_ddr_address_map_size *ddr_ptrs_size;
	struct ia_css_isp_parameters *params;

	assert(stream != NULL);
	IA_CSS_ENTER_PRIVATE("void");

	if (stream == NULL) {
		IA_CSS_LEAVE_ERR_PRIVATE(IA_CSS_ERR_INVALID_ARGUMENTS);
		return IA_CSS_ERR_INVALID_ARGUMENTS;
	}
	for (i = 0; i < curr_pipe->stream->num_pipes; i++) {
		struct ia_css_pipe *pipe;
		struct sh_css_ddr_address_map *cur_map;
		struct sh_css_ddr_address_map_size *cur_map_size;
		struct ia_css_isp_parameter_set_info isp_params_info;
		struct ia_css_pipeline *pipeline;
		struct ia_css_pipeline_stage *stage;

		enum sh_css_queue_id queue_id;

		(void)stage;
		pipe = curr_pipe->stream->pipes[i];
		pipeline = ia_css_pipe_get_pipeline(pipe);
		pipe_num = ia_css_pipe_get_pipe_num(pipe);
		isp_pipe_version = ia_css_pipe_get_isp_pipe_version(pipe);
		ia_css_pipeline_get_sp_thread_id(pipe_num, &thread_id);

#if defined(SH_CSS_ENABLE_PER_FRAME_PARAMS)
		ia_css_query_internal_queue_id(params->output_frame
						? IA_CSS_BUFFER_TYPE_PER_FRAME_PARAMETER_SET
						: IA_CSS_BUFFER_TYPE_PARAMETER_SET,
						thread_id, &queue_id);
#else
		ia_css_query_internal_queue_id(IA_CSS_BUFFER_TYPE_PARAMETER_SET, thread_id, &queue_id);
#endif
		if (!sh_css_sp_is_running()) {
			/* SP is not running. The queues are not valid */
			err = IA_CSS_ERR_RESOURCE_NOT_AVAILABLE;
			break;
		}