
static void _php_intlgregcal_constructor_body(INTERNAL_FUNCTION_PARAMETERS)
{
	zval		**tz_object	= NULL;
	zval		**args_a[6] = {0},
				***args		= &args_a[0];
	char		*locale		= NULL;
	}
	
	// instantion of ICU object
	GregorianCalendar *gcal = NULL;

	if (variant <= 2) {
		// From timezone and locale (0 to 2 arguments)
		TimeZone *tz = timezone_process_timezone_argument(tz_object, NULL,