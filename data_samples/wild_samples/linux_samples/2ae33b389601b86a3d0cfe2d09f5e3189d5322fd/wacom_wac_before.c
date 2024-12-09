		} else if (features->device_type == BTN_TOOL_PEN) {
			__set_bit(BTN_TOOL_RUBBER, input_dev->keybit);
			__set_bit(BTN_TOOL_PEN, input_dev->keybit);
			__set_bit(BTN_STYLUS, input_dev->keybit);
			__set_bit(BTN_STYLUS2, input_dev->keybit);
			input_set_abs_params(input_dev, ABS_DISTANCE, 0,
					      features->distance_max,
					      0, 0);
		}
		break;
	}
	return 0;
}

static const struct wacom_features wacom_features_0x00 =
	{ "Wacom Penpartner",     WACOM_PKGLEN_PENPRTN,    5040,  3780,  255,
	  0, PENPARTNER, WACOM_PENPRTN_RES, WACOM_PENPRTN_RES };
static const struct wacom_features wacom_features_0x10 =
	{ "Wacom Graphire",       WACOM_PKGLEN_GRAPHIRE,  10206,  7422,  511,
	  63, GRAPHIRE, WACOM_GRAPHIRE_RES, WACOM_GRAPHIRE_RES };
static const struct wacom_features wacom_features_0x11 =
	{ "Wacom Graphire2 4x5",  WACOM_PKGLEN_GRAPHIRE,  10206,  7422,  511,
	  63, GRAPHIRE, WACOM_GRAPHIRE_RES, WACOM_GRAPHIRE_RES };
static const struct wacom_features wacom_features_0x12 =
	{ "Wacom Graphire2 5x7",  WACOM_PKGLEN_GRAPHIRE,  13918, 10206,  511,
	  63, GRAPHIRE, WACOM_GRAPHIRE_RES, WACOM_GRAPHIRE_RES };
static const struct wacom_features wacom_features_0x13 =
	{ "Wacom Graphire3",      WACOM_PKGLEN_GRAPHIRE,  10208,  7424,  511,
	  63, GRAPHIRE, WACOM_GRAPHIRE_RES, WACOM_GRAPHIRE_RES };
static const struct wacom_features wacom_features_0x14 =
	{ "Wacom Graphire3 6x8",  WACOM_PKGLEN_GRAPHIRE,  16704, 12064,  511,
	  63, GRAPHIRE, WACOM_GRAPHIRE_RES, WACOM_GRAPHIRE_RES };
static const struct wacom_features wacom_features_0x15 =
	{ "Wacom Graphire4 4x5",  WACOM_PKGLEN_GRAPHIRE,  10208,  7424,  511,
	  63, WACOM_G4, WACOM_GRAPHIRE_RES, WACOM_GRAPHIRE_RES };
static const struct wacom_features wacom_features_0x16 =
	{ "Wacom Graphire4 6x8",  WACOM_PKGLEN_GRAPHIRE,  16704, 12064,  511,
	  63, WACOM_G4, WACOM_GRAPHIRE_RES, WACOM_GRAPHIRE_RES };
static const struct wacom_features wacom_features_0x17 =
	{ "Wacom BambooFun 4x5",  WACOM_PKGLEN_BBFUN,     14760,  9225,  511,
	  63, WACOM_MO, WACOM_INTUOS_RES, WACOM_INTUOS_RES };
static const struct wacom_features wacom_features_0x18 =
	{ "Wacom BambooFun 6x8",  WACOM_PKGLEN_BBFUN,     21648, 13530,  511,
	  63, WACOM_MO, WACOM_INTUOS_RES, WACOM_INTUOS_RES };
static const struct wacom_features wacom_features_0x19 =
	{ "Wacom Bamboo1 Medium", WACOM_PKGLEN_GRAPHIRE,  16704, 12064,  511,
	  63, GRAPHIRE, WACOM_GRAPHIRE_RES, WACOM_GRAPHIRE_RES };
static const struct wacom_features wacom_features_0x60 =
	{ "Wacom Volito",         WACOM_PKGLEN_GRAPHIRE,   5104,  3712,  511,
	  63, GRAPHIRE, WACOM_VOLITO_RES, WACOM_VOLITO_RES };
static const struct wacom_features wacom_features_0x61 =
	{ "Wacom PenStation2",    WACOM_PKGLEN_GRAPHIRE,   3250,  2320,  255,
	  63, GRAPHIRE, WACOM_VOLITO_RES, WACOM_VOLITO_RES };
static const struct wacom_features wacom_features_0x62 =
	{ "Wacom Volito2 4x5",    WACOM_PKGLEN_GRAPHIRE,   5104,  3712,  511,
	  63, GRAPHIRE, WACOM_VOLITO_RES, WACOM_VOLITO_RES };
static const struct wacom_features wacom_features_0x63 =
	{ "Wacom Volito2 2x3",    WACOM_PKGLEN_GRAPHIRE,   3248,  2320,  511,
	  63, GRAPHIRE, WACOM_VOLITO_RES, WACOM_VOLITO_RES };
static const struct wacom_features wacom_features_0x64 =
	{ "Wacom PenPartner2",    WACOM_PKGLEN_GRAPHIRE,   3250,  2320,  511,
	  63, GRAPHIRE, WACOM_VOLITO_RES, WACOM_VOLITO_RES };
static const struct wacom_features wacom_features_0x65 =
	{ "Wacom Bamboo",         WACOM_PKGLEN_BBFUN,     14760,  9225,  511,
	  63, WACOM_MO, WACOM_INTUOS_RES, WACOM_INTUOS_RES };
static const struct wacom_features wacom_features_0x69 =
	{ "Wacom Bamboo1",        WACOM_PKGLEN_GRAPHIRE,   5104,  3712,  511,
	  63, GRAPHIRE, WACOM_PENPRTN_RES, WACOM_PENPRTN_RES };
static const struct wacom_features wacom_features_0x6A =
	{ "Wacom Bamboo1 4x6",    WACOM_PKGLEN_GRAPHIRE,  14760,  9225, 1023,
	  63, GRAPHIRE, WACOM_INTUOS_RES, WACOM_INTUOS_RES };
static const struct wacom_features wacom_features_0x6B =
	{ "Wacom Bamboo1 5x8",    WACOM_PKGLEN_GRAPHIRE,  21648, 13530, 1023,
	  63, GRAPHIRE, WACOM_INTUOS_RES, WACOM_INTUOS_RES };
static const struct wacom_features wacom_features_0x20 =
	{ "Wacom Intuos 4x5",     WACOM_PKGLEN_INTUOS,    12700, 10600, 1023,
	  31, INTUOS, WACOM_INTUOS_RES, WACOM_INTUOS_RES };
static const struct wacom_features wacom_features_0x21 =
	{ "Wacom Intuos 6x8",     WACOM_PKGLEN_INTUOS,    20320, 16240, 1023,
	  31, INTUOS, WACOM_INTUOS_RES, WACOM_INTUOS_RES };
static const struct wacom_features wacom_features_0x22 =
	{ "Wacom Intuos 9x12",    WACOM_PKGLEN_INTUOS,    30480, 24060, 1023,
	  31, INTUOS, WACOM_INTUOS_RES, WACOM_INTUOS_RES };
static const struct wacom_features wacom_features_0x23 =
	{ "Wacom Intuos 12x12",   WACOM_PKGLEN_INTUOS,    30480, 31680, 1023,
	  31, INTUOS, WACOM_INTUOS_RES, WACOM_INTUOS_RES };
static const struct wacom_features wacom_features_0x24 =
	{ "Wacom Intuos 12x18",   WACOM_PKGLEN_INTUOS,    45720, 31680, 1023,
	  31, INTUOS, WACOM_INTUOS_RES, WACOM_INTUOS_RES };
static const struct wacom_features wacom_features_0x30 =
	{ "Wacom PL400",          WACOM_PKGLEN_GRAPHIRE,   5408,  4056,  255,
	  0, PL, WACOM_PL_RES, WACOM_PL_RES };
static const struct wacom_features wacom_features_0x31 =
	{ "Wacom PL500",          WACOM_PKGLEN_GRAPHIRE,   6144,  4608,  255,
	  0, PL, WACOM_PL_RES, WACOM_PL_RES };
static const struct wacom_features wacom_features_0x32 =
	{ "Wacom PL600",          WACOM_PKGLEN_GRAPHIRE,   6126,  4604,  255,
	  0, PL, WACOM_PL_RES, WACOM_PL_RES };
static const struct wacom_features wacom_features_0x33 =
	{ "Wacom PL600SX",        WACOM_PKGLEN_GRAPHIRE,   6260,  5016,  255,
	  0, PL, WACOM_PL_RES, WACOM_PL_RES };
static const struct wacom_features wacom_features_0x34 =
	{ "Wacom PL550",          WACOM_PKGLEN_GRAPHIRE,   6144,  4608,  511,
	  0, PL, WACOM_PL_RES, WACOM_PL_RES };
static const struct wacom_features wacom_features_0x35 =
	{ "Wacom PL800",          WACOM_PKGLEN_GRAPHIRE,   7220,  5780,  511,
	  0, PL, WACOM_PL_RES, WACOM_PL_RES };
static const struct wacom_features wacom_features_0x37 =
	{ "Wacom PL700",          WACOM_PKGLEN_GRAPHIRE,   6758,  5406,  511,
	  0, PL, WACOM_PL_RES, WACOM_PL_RES };
static const struct wacom_features wacom_features_0x38 =
	{ "Wacom PL510",          WACOM_PKGLEN_GRAPHIRE,   6282,  4762,  511,
	  0, PL, WACOM_PL_RES, WACOM_PL_RES };
static const struct wacom_features wacom_features_0x39 =
	{ "Wacom DTU710",         WACOM_PKGLEN_GRAPHIRE,  34080, 27660,  511,
	  0, PL, WACOM_PL_RES, WACOM_PL_RES };
static const struct wacom_features wacom_features_0xC4 =
	{ "Wacom DTF521",         WACOM_PKGLEN_GRAPHIRE,   6282,  4762,  511,
	  0, PL, WACOM_PL_RES, WACOM_PL_RES };
static const struct wacom_features wacom_features_0xC0 =
	{ "Wacom DTF720",         WACOM_PKGLEN_GRAPHIRE,   6858,  5506,  511,
	  0, PL, WACOM_PL_RES, WACOM_PL_RES };
static const struct wacom_features wacom_features_0xC2 =
	{ "Wacom DTF720a",        WACOM_PKGLEN_GRAPHIRE,   6858,  5506,  511,
	  0, PL, WACOM_PL_RES, WACOM_PL_RES };
static const struct wacom_features wacom_features_0x03 =
	{ "Wacom Cintiq Partner", WACOM_PKGLEN_GRAPHIRE,  20480, 15360,  511,
	  0, PTU, WACOM_PL_RES, WACOM_PL_RES };
static const struct wacom_features wacom_features_0x41 =
	{ "Wacom Intuos2 4x5",    WACOM_PKGLEN_INTUOS,    12700, 10600, 1023,
	  31, INTUOS, WACOM_INTUOS_RES, WACOM_INTUOS_RES };
static const struct wacom_features wacom_features_0x42 =
	{ "Wacom Intuos2 6x8",    WACOM_PKGLEN_INTUOS,    20320, 16240, 1023,
	  31, INTUOS, WACOM_INTUOS_RES, WACOM_INTUOS_RES };
static const struct wacom_features wacom_features_0x43 =
	{ "Wacom Intuos2 9x12",   WACOM_PKGLEN_INTUOS,    30480, 24060, 1023,
	  31, INTUOS, WACOM_INTUOS_RES, WACOM_INTUOS_RES };
static const struct wacom_features wacom_features_0x44 =
	{ "Wacom Intuos2 12x12",  WACOM_PKGLEN_INTUOS,    30480, 31680, 1023,
	  31, INTUOS, WACOM_INTUOS_RES, WACOM_INTUOS_RES };
static const struct wacom_features wacom_features_0x45 =
	{ "Wacom Intuos2 12x18",  WACOM_PKGLEN_INTUOS,    45720, 31680, 1023,
	  31, INTUOS, WACOM_INTUOS_RES, WACOM_INTUOS_RES };
static const struct wacom_features wacom_features_0xB0 =
	{ "Wacom Intuos3 4x5",    WACOM_PKGLEN_INTUOS,    25400, 20320, 1023,
	  63, INTUOS3S, WACOM_INTUOS3_RES, WACOM_INTUOS3_RES };
static const struct wacom_features wacom_features_0xB1 =
	{ "Wacom Intuos3 6x8",    WACOM_PKGLEN_INTUOS,    40640, 30480, 1023,
	  63, INTUOS3, WACOM_INTUOS3_RES, WACOM_INTUOS3_RES };
static const struct wacom_features wacom_features_0xB2 =
	{ "Wacom Intuos3 9x12",   WACOM_PKGLEN_INTUOS,    60960, 45720, 1023,
	  63, INTUOS3, WACOM_INTUOS3_RES, WACOM_INTUOS3_RES };
static const struct wacom_features wacom_features_0xB3 =
	{ "Wacom Intuos3 12x12",  WACOM_PKGLEN_INTUOS,    60960, 60960, 1023,
	  63, INTUOS3L, WACOM_INTUOS3_RES, WACOM_INTUOS3_RES };
static const struct wacom_features wacom_features_0xB4 =
	{ "Wacom Intuos3 12x19",  WACOM_PKGLEN_INTUOS,    97536, 60960, 1023,
	  63, INTUOS3L, WACOM_INTUOS3_RES, WACOM_INTUOS3_RES };
static const struct wacom_features wacom_features_0xB5 =
	{ "Wacom Intuos3 6x11",   WACOM_PKGLEN_INTUOS,    54204, 31750, 1023,
	  63, INTUOS3, WACOM_INTUOS3_RES, WACOM_INTUOS3_RES };
static const struct wacom_features wacom_features_0xB7 =
	{ "Wacom Intuos3 4x6",    WACOM_PKGLEN_INTUOS,    31496, 19685, 1023,
	  63, INTUOS3S, WACOM_INTUOS3_RES, WACOM_INTUOS3_RES };
static const struct wacom_features wacom_features_0xB8 =
	{ "Wacom Intuos4 4x6",    WACOM_PKGLEN_INTUOS,    31496, 19685, 2047,
	  63, INTUOS4S, WACOM_INTUOS3_RES, WACOM_INTUOS3_RES };
static const struct wacom_features wacom_features_0xB9 =
	{ "Wacom Intuos4 6x9",    WACOM_PKGLEN_INTUOS,    44704, 27940, 2047,
	  63, INTUOS4, WACOM_INTUOS3_RES, WACOM_INTUOS3_RES };
static const struct wacom_features wacom_features_0xBA =
	{ "Wacom Intuos4 8x13",   WACOM_PKGLEN_INTUOS,    65024, 40640, 2047,
	  63, INTUOS4L, WACOM_INTUOS3_RES, WACOM_INTUOS3_RES };
static const struct wacom_features wacom_features_0xBB =
	{ "Wacom Intuos4 12x19",  WACOM_PKGLEN_INTUOS,    97536, 60960, 2047,
	  63, INTUOS4L, WACOM_INTUOS3_RES, WACOM_INTUOS3_RES };
static const struct wacom_features wacom_features_0xBC =
	{ "Wacom Intuos4 WL",     WACOM_PKGLEN_INTUOS,    40840, 25400, 2047,
	  63, INTUOS4, WACOM_INTUOS3_RES, WACOM_INTUOS3_RES };
static const struct wacom_features wacom_features_0x26 =
	{ "Wacom Intuos5 touch S", WACOM_PKGLEN_INTUOS,  31496, 19685, 2047,
	  63, INTUOS5S, WACOM_INTUOS3_RES, WACOM_INTUOS3_RES,
	  .touch_max = 16 };
static const struct wacom_features wacom_features_0x27 =
	{ "Wacom Intuos5 touch M", WACOM_PKGLEN_INTUOS,  44704, 27940, 2047,
	  63, INTUOS5, WACOM_INTUOS3_RES, WACOM_INTUOS3_RES,
	  .touch_max = 16 };
static const struct wacom_features wacom_features_0x28 =
	{ "Wacom Intuos5 touch L", WACOM_PKGLEN_INTUOS, 65024, 40640, 2047,
	  63, INTUOS5L, WACOM_INTUOS3_RES, WACOM_INTUOS3_RES,
	  .touch_max = 16 };
static const struct wacom_features wacom_features_0x29 =
	{ "Wacom Intuos5 S", WACOM_PKGLEN_INTUOS,  31496, 19685, 2047,
	  63, INTUOS5S, WACOM_INTUOS3_RES, WACOM_INTUOS3_RES };
static const struct wacom_features wacom_features_0x2A =
	{ "Wacom Intuos5 M", WACOM_PKGLEN_INTUOS,  44704, 27940, 2047,
	  63, INTUOS5, WACOM_INTUOS3_RES, WACOM_INTUOS3_RES };
static const struct wacom_features wacom_features_0xF4 =
	{ "Wacom Cintiq 24HD",       WACOM_PKGLEN_INTUOS,   104480, 65600, 2047,
	  63, WACOM_24HD, WACOM_INTUOS3_RES, WACOM_INTUOS3_RES };
static const struct wacom_features wacom_features_0xF8 =
	{ "Wacom Cintiq 24HD touch", WACOM_PKGLEN_INTUOS,   104480, 65600, 2047, /* Pen */
	  63, WACOM_24HD, WACOM_INTUOS3_RES, WACOM_INTUOS3_RES, .oVid = USB_VENDOR_ID_WACOM, .oPid = 0xf6 };
static const struct wacom_features wacom_features_0xF6 =
	{ "Wacom Cintiq 24HD touch", .type = WACOM_24HDT, /* Touch */
	  .oVid = USB_VENDOR_ID_WACOM, .oPid = 0xf8, .touch_max = 10 };
static const struct wacom_features wacom_features_0x3F =
	{ "Wacom Cintiq 21UX",    WACOM_PKGLEN_INTUOS,    87200, 65600, 1023,
	  63, CINTIQ, WACOM_INTUOS3_RES, WACOM_INTUOS3_RES };
static const struct wacom_features wacom_features_0xC5 =
	{ "Wacom Cintiq 20WSX",   WACOM_PKGLEN_INTUOS,    86680, 54180, 1023,
	  63, WACOM_BEE, WACOM_INTUOS3_RES, WACOM_INTUOS3_RES };
static const struct wacom_features wacom_features_0xC6 =
	{ "Wacom Cintiq 12WX",    WACOM_PKGLEN_INTUOS,    53020, 33440, 1023,
	  63, WACOM_BEE, WACOM_INTUOS3_RES, WACOM_INTUOS3_RES };
static const struct wacom_features wacom_features_0xC7 =
	{ "Wacom DTU1931",        WACOM_PKGLEN_GRAPHIRE,  37832, 30305,  511,
	  0, PL, WACOM_INTUOS_RES, WACOM_INTUOS_RES };
static const struct wacom_features wacom_features_0xCE =
	{ "Wacom DTU2231",        WACOM_PKGLEN_GRAPHIRE,  47864, 27011,  511,
	  0, DTU, WACOM_INTUOS_RES, WACOM_INTUOS_RES };
static const struct wacom_features wacom_features_0xF0 =
	{ "Wacom DTU1631",        WACOM_PKGLEN_GRAPHIRE,  34623, 19553,  511,
	  0, DTU, WACOM_INTUOS_RES, WACOM_INTUOS_RES };
static const struct wacom_features wacom_features_0x59 = /* Pen */
	{ "Wacom DTH2242",        WACOM_PKGLEN_INTUOS,    95840, 54260, 2047,
	  63, DTK, WACOM_INTUOS3_RES, WACOM_INTUOS3_RES,
	  .oVid = USB_VENDOR_ID_WACOM, .oPid = 0x5D };
static const struct wacom_features wacom_features_0x5D = /* Touch */
	{ "Wacom DTH2242",       .type = WACOM_24HDT,
	  .oVid = USB_VENDOR_ID_WACOM, .oPid = 0x59, .touch_max = 10 };
static const struct wacom_features wacom_features_0xCC =
	{ "Wacom Cintiq 21UX2",   WACOM_PKGLEN_INTUOS,    87200, 65600, 2047,
	  63, WACOM_21UX2, WACOM_INTUOS3_RES, WACOM_INTUOS3_RES };
static const struct wacom_features wacom_features_0xFA =
	{ "Wacom Cintiq 22HD",    WACOM_PKGLEN_INTUOS,    95840, 54260, 2047,
	  63, WACOM_22HD, WACOM_INTUOS3_RES, WACOM_INTUOS3_RES };
static const struct wacom_features wacom_features_0x90 =
	{ "Wacom ISDv4 90",       WACOM_PKGLEN_GRAPHIRE,  26202, 16325,  255,
	  0, TABLETPC, WACOM_INTUOS_RES, WACOM_INTUOS_RES };
static const struct wacom_features wacom_features_0x93 =
	{ "Wacom ISDv4 93",       WACOM_PKGLEN_GRAPHIRE,  26202, 16325,  255,
	  0, TABLETPC, WACOM_INTUOS_RES, WACOM_INTUOS_RES };
static const struct wacom_features wacom_features_0x97 =
	{ "Wacom ISDv4 97",       WACOM_PKGLEN_GRAPHIRE,  26202, 16325,  511,
	  0, TABLETPC, WACOM_INTUOS_RES, WACOM_INTUOS_RES };
static const struct wacom_features wacom_features_0x9A =
	{ "Wacom ISDv4 9A",       WACOM_PKGLEN_GRAPHIRE,  26202, 16325,  255,
	  0, TABLETPC, WACOM_INTUOS_RES, WACOM_INTUOS_RES };
static const struct wacom_features wacom_features_0x9F =
	{ "Wacom ISDv4 9F",       WACOM_PKGLEN_GRAPHIRE,  26202, 16325,  255,
	  0, TABLETPC, WACOM_INTUOS_RES, WACOM_INTUOS_RES };
static const struct wacom_features wacom_features_0xE2 =
	{ "Wacom ISDv4 E2",       WACOM_PKGLEN_TPC2FG,    26202, 16325,  255,
	  0, TABLETPC2FG, WACOM_INTUOS_RES, WACOM_INTUOS_RES,
	  .touch_max = 2 };
static const struct wacom_features wacom_features_0xE3 =
	{ "Wacom ISDv4 E3",       WACOM_PKGLEN_TPC2FG,    26202, 16325,  255,
	  0, TABLETPC2FG, WACOM_INTUOS_RES, WACOM_INTUOS_RES,
	  .touch_max = 2 };
static const struct wacom_features wacom_features_0xE5 =
	{ "Wacom ISDv4 E5",       WACOM_PKGLEN_MTOUCH,    26202, 16325,  255,
	  0, MTSCREEN, WACOM_INTUOS_RES, WACOM_INTUOS_RES };
static const struct wacom_features wacom_features_0xE6 =
	{ "Wacom ISDv4 E6",       WACOM_PKGLEN_TPC2FG,    27760, 15694,  255,
	  0, TABLETPC2FG, WACOM_INTUOS_RES, WACOM_INTUOS_RES,
	.touch_max = 2 };
static const struct wacom_features wacom_features_0xEC =
	{ "Wacom ISDv4 EC",       WACOM_PKGLEN_GRAPHIRE,  25710, 14500,  255,
	  0, TABLETPC,    WACOM_INTUOS_RES, WACOM_INTUOS_RES };
static const struct wacom_features wacom_features_0xED =
	{ "Wacom ISDv4 ED",       WACOM_PKGLEN_GRAPHIRE,  26202, 16325,  255,
	  0, TABLETPCE, WACOM_INTUOS_RES, WACOM_INTUOS_RES };
static const struct wacom_features wacom_features_0xEF =
	{ "Wacom ISDv4 EF",       WACOM_PKGLEN_GRAPHIRE,  26202, 16325,  255,
	  0, TABLETPC, WACOM_INTUOS_RES, WACOM_INTUOS_RES };
static const struct wacom_features wacom_features_0x100 =
	{ "Wacom ISDv4 100",      WACOM_PKGLEN_MTTPC,     26202, 16325,  255,
	  0, MTTPC, WACOM_INTUOS_RES, WACOM_INTUOS_RES };
static const struct wacom_features wacom_features_0x101 =
	{ "Wacom ISDv4 101",      WACOM_PKGLEN_MTTPC,     26202, 16325,  255,
	  0, MTTPC, WACOM_INTUOS_RES, WACOM_INTUOS_RES };
static const struct wacom_features wacom_features_0x4001 =
	{ "Wacom ISDv4 4001",      WACOM_PKGLEN_MTTPC,     26202, 16325,  255,
	  0, MTTPC, WACOM_INTUOS_RES, WACOM_INTUOS_RES };
static const struct wacom_features wacom_features_0x47 =
	{ "Wacom Intuos2 6x8",    WACOM_PKGLEN_INTUOS,    20320, 16240, 1023,
	  31, INTUOS, WACOM_INTUOS_RES, WACOM_INTUOS_RES };
static const struct wacom_features wacom_features_0x84 =
	{ "Wacom Wireless Receiver", WACOM_PKGLEN_WIRELESS, 0, 0, 0,
	  0, WIRELESS, 0, 0, .touch_max = 16 };
static const struct wacom_features wacom_features_0xD0 =
	{ "Wacom Bamboo 2FG",     WACOM_PKGLEN_BBFUN,     14720,  9200, 1023,
	  31, BAMBOO_PT, WACOM_INTUOS_RES, WACOM_INTUOS_RES,
	  .touch_max = 2 };
static const struct wacom_features wacom_features_0xD1 =
	{ "Wacom Bamboo 2FG 4x5", WACOM_PKGLEN_BBFUN,     14720,  9200, 1023,
	  31, BAMBOO_PT, WACOM_INTUOS_RES, WACOM_INTUOS_RES,
	  .touch_max = 2 };
static const struct wacom_features wacom_features_0xD2 =
	{ "Wacom Bamboo Craft",   WACOM_PKGLEN_BBFUN,     14720,  9200, 1023,
	  31, BAMBOO_PT, WACOM_INTUOS_RES, WACOM_INTUOS_RES,
	  .touch_max = 2 };
static const struct wacom_features wacom_features_0xD3 =
	{ "Wacom Bamboo 2FG 6x8", WACOM_PKGLEN_BBFUN,     21648, 13700, 1023,
	  31, BAMBOO_PT, WACOM_INTUOS_RES, WACOM_INTUOS_RES,
	  .touch_max = 2 };
static const struct wacom_features wacom_features_0xD4 =
	{ "Wacom Bamboo Pen",     WACOM_PKGLEN_BBFUN,     14720,  9200, 1023,
	  31, BAMBOO_PT, WACOM_INTUOS_RES, WACOM_INTUOS_RES };
static const struct wacom_features wacom_features_0xD5 =
	{ "Wacom Bamboo Pen 6x8",     WACOM_PKGLEN_BBFUN, 21648, 13700, 1023,
	  31, BAMBOO_PT, WACOM_INTUOS_RES, WACOM_INTUOS_RES };
static const struct wacom_features wacom_features_0xD6 =
	{ "Wacom BambooPT 2FG 4x5", WACOM_PKGLEN_BBFUN,   14720,  9200, 1023,
	  31, BAMBOO_PT, WACOM_INTUOS_RES, WACOM_INTUOS_RES,
	  .touch_max = 2 };
static const struct wacom_features wacom_features_0xD7 =
	{ "Wacom BambooPT 2FG Small", WACOM_PKGLEN_BBFUN, 14720,  9200, 1023,
	  31, BAMBOO_PT, WACOM_INTUOS_RES, WACOM_INTUOS_RES,
	  .touch_max = 2 };
static const struct wacom_features wacom_features_0xD8 =
	{ "Wacom Bamboo Comic 2FG", WACOM_PKGLEN_BBFUN,   21648, 13700, 1023,
	  31, BAMBOO_PT, WACOM_INTUOS_RES, WACOM_INTUOS_RES,
	  .touch_max = 2 };
static const struct wacom_features wacom_features_0xDA =
	{ "Wacom Bamboo 2FG 4x5 SE", WACOM_PKGLEN_BBFUN,  14720,  9200, 1023,
	  31, BAMBOO_PT, WACOM_INTUOS_RES, WACOM_INTUOS_RES,
	  .touch_max = 2 };
static struct wacom_features wacom_features_0xDB =
	{ "Wacom Bamboo 2FG 6x8 SE", WACOM_PKGLEN_BBFUN,  21648, 13700, 1023,
	  31, BAMBOO_PT, WACOM_INTUOS_RES, WACOM_INTUOS_RES,
	  .touch_max = 2 };
static const struct wacom_features wacom_features_0xDD =
        { "Wacom Bamboo Connect", WACOM_PKGLEN_BBPEN,     14720,  9200, 1023,
          31, BAMBOO_PT, WACOM_INTUOS_RES, WACOM_INTUOS_RES };
static const struct wacom_features wacom_features_0xDE =
        { "Wacom Bamboo 16FG 4x5", WACOM_PKGLEN_BBPEN,    14720,  9200, 1023,
	  31, BAMBOO_PT, WACOM_INTUOS_RES, WACOM_INTUOS_RES,
	  .touch_max = 16 };
static const struct wacom_features wacom_features_0xDF =
        { "Wacom Bamboo 16FG 6x8", WACOM_PKGLEN_BBPEN,    21648, 13700, 1023,
	  31, BAMBOO_PT, WACOM_INTUOS_RES, WACOM_INTUOS_RES,
	  .touch_max = 16 };
static const struct wacom_features wacom_features_0x6004 =
	{ "ISD-V4",               WACOM_PKGLEN_GRAPHIRE,  12800,  8000,  255,
	  0, TABLETPC, WACOM_INTUOS_RES, WACOM_INTUOS_RES };

#define USB_DEVICE_WACOM(prod)					\
	USB_DEVICE(USB_VENDOR_ID_WACOM, prod),			\
	.driver_info = (kernel_ulong_t)&wacom_features_##prod

#define USB_DEVICE_DETAILED(prod, class, sub, proto)			\
	USB_DEVICE_AND_INTERFACE_INFO(USB_VENDOR_ID_WACOM, prod, class,	\
				      sub, proto),			\
	.driver_info = (kernel_ulong_t)&wacom_features_##prod

#define USB_DEVICE_LENOVO(prod)					\
	USB_DEVICE(USB_VENDOR_ID_LENOVO, prod),			\
	.driver_info = (kernel_ulong_t)&wacom_features_##prod

const struct usb_device_id wacom_ids[] = {
	{ USB_DEVICE_WACOM(0x00) },
	{ USB_DEVICE_WACOM(0x10) },
	{ USB_DEVICE_WACOM(0x11) },
	{ USB_DEVICE_WACOM(0x12) },
	{ USB_DEVICE_WACOM(0x13) },
	{ USB_DEVICE_WACOM(0x14) },
	{ USB_DEVICE_WACOM(0x15) },
	{ USB_DEVICE_WACOM(0x16) },
	{ USB_DEVICE_WACOM(0x17) },
	{ USB_DEVICE_WACOM(0x18) },
	{ USB_DEVICE_WACOM(0x19) },
	{ USB_DEVICE_WACOM(0x60) },
	{ USB_DEVICE_WACOM(0x61) },
	{ USB_DEVICE_WACOM(0x62) },
	{ USB_DEVICE_WACOM(0x63) },
	{ USB_DEVICE_WACOM(0x64) },
	{ USB_DEVICE_WACOM(0x65) },
	{ USB_DEVICE_WACOM(0x69) },
	{ USB_DEVICE_WACOM(0x6A) },
	{ USB_DEVICE_WACOM(0x6B) },
	{ USB_DEVICE_WACOM(0x20) },
	{ USB_DEVICE_WACOM(0x21) },
	{ USB_DEVICE_WACOM(0x22) },
	{ USB_DEVICE_WACOM(0x23) },
	{ USB_DEVICE_WACOM(0x24) },
	{ USB_DEVICE_WACOM(0x30) },
	{ USB_DEVICE_WACOM(0x31) },
	{ USB_DEVICE_WACOM(0x32) },
	{ USB_DEVICE_WACOM(0x33) },
	{ USB_DEVICE_WACOM(0x34) },
	{ USB_DEVICE_WACOM(0x35) },
	{ USB_DEVICE_WACOM(0x37) },
	{ USB_DEVICE_WACOM(0x38) },
	{ USB_DEVICE_WACOM(0x39) },
	{ USB_DEVICE_WACOM(0xC4) },
	{ USB_DEVICE_WACOM(0xC0) },
	{ USB_DEVICE_WACOM(0xC2) },
	{ USB_DEVICE_WACOM(0x03) },
	{ USB_DEVICE_WACOM(0x41) },
	{ USB_DEVICE_WACOM(0x42) },
	{ USB_DEVICE_WACOM(0x43) },
	{ USB_DEVICE_WACOM(0x44) },
	{ USB_DEVICE_WACOM(0x45) },
	{ USB_DEVICE_WACOM(0x59) },
	{ USB_DEVICE_WACOM(0x5D) },
	{ USB_DEVICE_WACOM(0xB0) },
	{ USB_DEVICE_WACOM(0xB1) },
	{ USB_DEVICE_WACOM(0xB2) },
	{ USB_DEVICE_WACOM(0xB3) },
	{ USB_DEVICE_WACOM(0xB4) },
	{ USB_DEVICE_WACOM(0xB5) },
	{ USB_DEVICE_WACOM(0xB7) },
	{ USB_DEVICE_WACOM(0xB8) },
	{ USB_DEVICE_WACOM(0xB9) },
	{ USB_DEVICE_WACOM(0xBA) },
	{ USB_DEVICE_WACOM(0xBB) },
	{ USB_DEVICE_WACOM(0xBC) },
	{ USB_DEVICE_WACOM(0x26) },
	{ USB_DEVICE_WACOM(0x27) },
	{ USB_DEVICE_WACOM(0x28) },
	{ USB_DEVICE_WACOM(0x29) },
	{ USB_DEVICE_WACOM(0x2A) },
	{ USB_DEVICE_WACOM(0x3F) },
	{ USB_DEVICE_WACOM(0xC5) },
	{ USB_DEVICE_WACOM(0xC6) },
	{ USB_DEVICE_WACOM(0xC7) },
	/*
	 * DTU-2231 has two interfaces on the same configuration,
	 * only one is used.
	 */
	{ USB_DEVICE_DETAILED(0xCE, USB_CLASS_HID,
			      USB_INTERFACE_SUBCLASS_BOOT,
			      USB_INTERFACE_PROTOCOL_MOUSE) },
	{ USB_DEVICE_WACOM(0x84) },
	{ USB_DEVICE_WACOM(0xD0) },
	{ USB_DEVICE_WACOM(0xD1) },
	{ USB_DEVICE_WACOM(0xD2) },
	{ USB_DEVICE_WACOM(0xD3) },
	{ USB_DEVICE_WACOM(0xD4) },
	{ USB_DEVICE_WACOM(0xD5) },
	{ USB_DEVICE_WACOM(0xD6) },
	{ USB_DEVICE_WACOM(0xD7) },
	{ USB_DEVICE_WACOM(0xD8) },
	{ USB_DEVICE_WACOM(0xDA) },
	{ USB_DEVICE_WACOM(0xDB) },
	{ USB_DEVICE_WACOM(0xDD) },
	{ USB_DEVICE_WACOM(0xDE) },
	{ USB_DEVICE_WACOM(0xDF) },
	{ USB_DEVICE_WACOM(0xF0) },
	{ USB_DEVICE_WACOM(0xCC) },
	{ USB_DEVICE_WACOM(0x90) },
	{ USB_DEVICE_WACOM(0x93) },
	{ USB_DEVICE_WACOM(0x97) },
	{ USB_DEVICE_WACOM(0x9A) },
	{ USB_DEVICE_WACOM(0x9F) },
	{ USB_DEVICE_WACOM(0xE2) },
	{ USB_DEVICE_WACOM(0xE3) },
	{ USB_DEVICE_WACOM(0xE5) },
	{ USB_DEVICE_WACOM(0xE6) },
	{ USB_DEVICE_WACOM(0xEC) },
	{ USB_DEVICE_WACOM(0xED) },
	{ USB_DEVICE_WACOM(0xEF) },
	{ USB_DEVICE_WACOM(0x100) },
	{ USB_DEVICE_WACOM(0x101) },
	{ USB_DEVICE_WACOM(0x4001) },
	{ USB_DEVICE_WACOM(0x47) },
	{ USB_DEVICE_WACOM(0xF4) },
	{ USB_DEVICE_WACOM(0xF8) },
	{ USB_DEVICE_WACOM(0xF6) },
	{ USB_DEVICE_WACOM(0xFA) },
	{ USB_DEVICE_LENOVO(0x6004) },
	{ }
const struct usb_device_id wacom_ids[] = {
	{ USB_DEVICE_WACOM(0x00) },
	{ USB_DEVICE_WACOM(0x10) },
	{ USB_DEVICE_WACOM(0x11) },
	{ USB_DEVICE_WACOM(0x12) },
	{ USB_DEVICE_WACOM(0x13) },
	{ USB_DEVICE_WACOM(0x14) },
	{ USB_DEVICE_WACOM(0x15) },
	{ USB_DEVICE_WACOM(0x16) },
	{ USB_DEVICE_WACOM(0x17) },
	{ USB_DEVICE_WACOM(0x18) },
	{ USB_DEVICE_WACOM(0x19) },
	{ USB_DEVICE_WACOM(0x60) },
	{ USB_DEVICE_WACOM(0x61) },
	{ USB_DEVICE_WACOM(0x62) },
	{ USB_DEVICE_WACOM(0x63) },
	{ USB_DEVICE_WACOM(0x64) },
	{ USB_DEVICE_WACOM(0x65) },
	{ USB_DEVICE_WACOM(0x69) },
	{ USB_DEVICE_WACOM(0x6A) },
	{ USB_DEVICE_WACOM(0x6B) },
	{ USB_DEVICE_WACOM(0x20) },
	{ USB_DEVICE_WACOM(0x21) },
	{ USB_DEVICE_WACOM(0x22) },
	{ USB_DEVICE_WACOM(0x23) },
	{ USB_DEVICE_WACOM(0x24) },
	{ USB_DEVICE_WACOM(0x30) },
	{ USB_DEVICE_WACOM(0x31) },
	{ USB_DEVICE_WACOM(0x32) },
	{ USB_DEVICE_WACOM(0x33) },
	{ USB_DEVICE_WACOM(0x34) },
	{ USB_DEVICE_WACOM(0x35) },
	{ USB_DEVICE_WACOM(0x37) },
	{ USB_DEVICE_WACOM(0x38) },
	{ USB_DEVICE_WACOM(0x39) },
	{ USB_DEVICE_WACOM(0xC4) },
	{ USB_DEVICE_WACOM(0xC0) },
	{ USB_DEVICE_WACOM(0xC2) },
	{ USB_DEVICE_WACOM(0x03) },
	{ USB_DEVICE_WACOM(0x41) },
	{ USB_DEVICE_WACOM(0x42) },
	{ USB_DEVICE_WACOM(0x43) },
	{ USB_DEVICE_WACOM(0x44) },
	{ USB_DEVICE_WACOM(0x45) },
	{ USB_DEVICE_WACOM(0x59) },
	{ USB_DEVICE_WACOM(0x5D) },
	{ USB_DEVICE_WACOM(0xB0) },
	{ USB_DEVICE_WACOM(0xB1) },
	{ USB_DEVICE_WACOM(0xB2) },
	{ USB_DEVICE_WACOM(0xB3) },
	{ USB_DEVICE_WACOM(0xB4) },
	{ USB_DEVICE_WACOM(0xB5) },
	{ USB_DEVICE_WACOM(0xB7) },
	{ USB_DEVICE_WACOM(0xB8) },
	{ USB_DEVICE_WACOM(0xB9) },
	{ USB_DEVICE_WACOM(0xBA) },
	{ USB_DEVICE_WACOM(0xBB) },
	{ USB_DEVICE_WACOM(0xBC) },
	{ USB_DEVICE_WACOM(0x26) },
	{ USB_DEVICE_WACOM(0x27) },
	{ USB_DEVICE_WACOM(0x28) },
	{ USB_DEVICE_WACOM(0x29) },
	{ USB_DEVICE_WACOM(0x2A) },
	{ USB_DEVICE_WACOM(0x3F) },
	{ USB_DEVICE_WACOM(0xC5) },
	{ USB_DEVICE_WACOM(0xC6) },
	{ USB_DEVICE_WACOM(0xC7) },
	/*
	 * DTU-2231 has two interfaces on the same configuration,
	 * only one is used.
	 */
	{ USB_DEVICE_DETAILED(0xCE, USB_CLASS_HID,
			      USB_INTERFACE_SUBCLASS_BOOT,
			      USB_INTERFACE_PROTOCOL_MOUSE) },
	{ USB_DEVICE_WACOM(0x84) },
	{ USB_DEVICE_WACOM(0xD0) },
	{ USB_DEVICE_WACOM(0xD1) },
	{ USB_DEVICE_WACOM(0xD2) },
	{ USB_DEVICE_WACOM(0xD3) },
	{ USB_DEVICE_WACOM(0xD4) },
	{ USB_DEVICE_WACOM(0xD5) },
	{ USB_DEVICE_WACOM(0xD6) },
	{ USB_DEVICE_WACOM(0xD7) },
	{ USB_DEVICE_WACOM(0xD8) },
	{ USB_DEVICE_WACOM(0xDA) },
	{ USB_DEVICE_WACOM(0xDB) },
	{ USB_DEVICE_WACOM(0xDD) },
	{ USB_DEVICE_WACOM(0xDE) },
	{ USB_DEVICE_WACOM(0xDF) },
	{ USB_DEVICE_WACOM(0xF0) },
	{ USB_DEVICE_WACOM(0xCC) },
	{ USB_DEVICE_WACOM(0x90) },
	{ USB_DEVICE_WACOM(0x93) },
	{ USB_DEVICE_WACOM(0x97) },
	{ USB_DEVICE_WACOM(0x9A) },
	{ USB_DEVICE_WACOM(0x9F) },
	{ USB_DEVICE_WACOM(0xE2) },
	{ USB_DEVICE_WACOM(0xE3) },
	{ USB_DEVICE_WACOM(0xE5) },
	{ USB_DEVICE_WACOM(0xE6) },
	{ USB_DEVICE_WACOM(0xEC) },
	{ USB_DEVICE_WACOM(0xED) },
	{ USB_DEVICE_WACOM(0xEF) },
	{ USB_DEVICE_WACOM(0x100) },
	{ USB_DEVICE_WACOM(0x101) },
	{ USB_DEVICE_WACOM(0x4001) },
	{ USB_DEVICE_WACOM(0x47) },
	{ USB_DEVICE_WACOM(0xF4) },
	{ USB_DEVICE_WACOM(0xF8) },
	{ USB_DEVICE_WACOM(0xF6) },
	{ USB_DEVICE_WACOM(0xFA) },
	{ USB_DEVICE_LENOVO(0x6004) },
	{ }
};
MODULE_DEVICE_TABLE(usb, wacom_ids);