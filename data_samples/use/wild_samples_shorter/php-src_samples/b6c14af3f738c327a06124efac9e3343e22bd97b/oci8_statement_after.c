   Fetch implicit result set statement resource */
php_oci_statement *php_oci_get_implicit_resultset(php_oci_statement *statement TSRMLS_DC)
{
#if (OCI_MAJOR_VERSION < 12)
	php_error_docref(NULL TSRMLS_CC, E_WARNING, "Implicit results are available in Oracle Database 12c onwards");
	return NULL;
#else
	void *result;
	ub4   rtype;
	php_oci_statement *statement2;  /* implicit result set statement handle */

	PHP_OCI_CALL_RETURN(OCISTMTGETNEXTRESULT, statement->errcode, OCIStmtGetNextResult, (statement->stmt, statement->err, &result, &rtype, OCI_DEFAULT));
	if (statement->errcode == OCI_NO_DATA) {
		return NULL;
	}