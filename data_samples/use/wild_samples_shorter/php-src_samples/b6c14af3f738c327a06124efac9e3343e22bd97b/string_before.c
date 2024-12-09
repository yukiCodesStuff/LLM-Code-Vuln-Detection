static int php_similar_char(const char *txt1, int len1, const char *txt2, int len2)
{
	int sum;
	int pos1, pos2, max;

	php_similar_str(txt1, len1, txt2, len2, &pos1, &pos2, &max);
	if ((sum = max)) {
		if (pos1 && pos2) {
	char *tbuf, *buf, *p, *tp, *rp, c, lc;
	int br, i=0, depth=0, in_q = 0;
	int state = 0, pos;
	char *allow_free;

	if (stateptr)
		state = *stateptr;
