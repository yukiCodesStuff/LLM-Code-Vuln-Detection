/* {{{ grapheme_strpos_utf16 - strrpos using utf16*/
int grapheme_strpos_utf16(unsigned char *haystack, int32_t haystack_len, unsigned char*needle, int32_t needle_len, int32_t offset, int32_t *puchar_pos, int f_ignore_case, int last TSRMLS_DC)
{
	UChar *uhaystack = NULL, *puhaystack, *uneedle = NULL;
	int32_t uhaystack_len = 0, uneedle_len = 0, char_pos, ret_pos, offset_pos = 0;
	unsigned char u_break_iterator_buffer[U_BRK_SAFECLONE_BUFFERSIZE];
	UBreakIterator* bi = NULL;
	UErrorCode status;