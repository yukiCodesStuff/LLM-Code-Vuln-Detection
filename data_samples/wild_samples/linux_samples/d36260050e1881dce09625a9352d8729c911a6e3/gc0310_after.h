static struct gc0310_reg const gc0310_VGA_30fps[] = {
	{GC0310_8BIT, 0xfe, 0x00},
	{GC0310_8BIT, 0x0d, 0x01}, //height
	{GC0310_8BIT, 0x0e, 0xf2}, // 0xf7 //height
	{GC0310_8BIT, 0x0f, 0x02}, //width
	{GC0310_8BIT, 0x10, 0x94}, // 0xa0 //height

	{GC0310_8BIT, 0x50, 0x01}, //crop enable
	{GC0310_8BIT, 0x51, 0x00},
	{GC0310_8BIT, 0x52, 0x00},
	{GC0310_8BIT, 0x53, 0x00},
	{GC0310_8BIT, 0x54, 0x01},
	{GC0310_8BIT, 0x55, 0x01}, //crop window height
	{GC0310_8BIT, 0x56, 0xf0},
	{GC0310_8BIT, 0x57, 0x02}, //crop window width
	{GC0310_8BIT, 0x58, 0x90},

	{GC0310_8BIT, 0xfe, 0x03},
	{GC0310_8BIT, 0x12, 0x90},//00 //04 //00 //04//00 //LWC[7:0]  //
	{GC0310_8BIT, 0x13, 0x02},//05 //05 //LWC[15:8]

	{GC0310_8BIT, 0xfe, 0x00},

	{GC0310_TOK_TERM, 0, 0},
};

static struct gc0310_resolution gc0310_res_preview[] = {
	{
		.desc = "gc0310_VGA_30fps",
		.width = 656, // 648,
		.height = 496, // 488,
		.fps = 30,
		//.pix_clk_freq = 73,
		.used = 0,
#if 0
		.pixels_per_line = 0x0314,
		.lines_per_frame = 0x0213,
#endif
		.bin_factor_x = 1,
		.bin_factor_y = 1,
		.bin_mode = 0,
		.skip_frames = 2,
		.regs = gc0310_VGA_30fps,
	},
};
#define N_RES_PREVIEW (ARRAY_SIZE(gc0310_res_preview))

static struct gc0310_resolution *gc0310_res = gc0310_res_preview;
static unsigned long N_RES = N_RES_PREVIEW;
#endif
