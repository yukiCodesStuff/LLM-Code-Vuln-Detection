				}
			case Formattable::kLong:
				{
					int32_t tInt32 = 0;
retry_klong:
					if (Z_TYPE_PP(elem) == IS_DOUBLE) {
						if (Z_DVAL_PP(elem) > (double)INT32_MAX ||
								Z_DVAL_PP(elem) < (double)INT32_MIN) {
				}
			case Formattable::kInt64:
				{
					int64_t tInt64 = 0;
retry_kint64:
					if (Z_TYPE_PP(elem) == IS_DOUBLE) {
						if (Z_DVAL_PP(elem) > (double)U_INT64_MAX ||
								Z_DVAL_PP(elem) < (double)U_INT64_MIN) {