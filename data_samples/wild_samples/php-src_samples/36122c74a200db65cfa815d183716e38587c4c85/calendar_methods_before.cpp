    if (U_FAILURE(status)) {
        uenum_close(uenum);
		intl_error_set(NULL, status, "intlcal_get_keyword_values_for_locale: "
			"error calling underlying method", 0 TSRMLS_CC);
        RETURN_FALSE;
    }

    StringEnumeration *se = new BugStringCharEnumeration(uenum);
#endif

	IntlIterator_from_StringEnumeration(se, return_value TSRMLS_CC);
}
#endif //ICU 4.2 only

U_CFUNC PHP_FUNCTION(intlcal_get_now)
{
	UErrorCode	status			= U_ZERO_ERROR;
	intl_error_reset(NULL TSRMLS_CC);

	if (zend_parse_parameters_none() == FAILURE) {
		intl_error_set(NULL, U_ILLEGAL_ARGUMENT_ERROR,
			"intlcal_get_now: bad arguments", 0 TSRMLS_CC);
		RETURN_FALSE;
	}

	RETURN_DOUBLE((double)Calendar::getNow());
}