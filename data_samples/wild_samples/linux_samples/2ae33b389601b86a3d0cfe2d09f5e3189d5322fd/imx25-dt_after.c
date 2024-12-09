static const char * const imx25_dt_board_compat[] __initconst = {
	"fsl,imx25",
	NULL
};

static void __init imx25_timer_init(void)
{
	mx25_clocks_init_dt();
}