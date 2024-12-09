/*
  $Id$
*/
/*
 * This code comes from John the Ripper password cracker, with reentrant
 * and crypt(3) interfaces added, but optimizations specific to password
 * cracking removed.
 *
 * Written by Solar Designer <solar at openwall.com> in 1998-2002 and
 * placed in the public domain.  Quick self-test added in 2011 and also
 * placed in the public domain.
 *
 * There's absolutely no warranty.
 *
#define __CONST __const
#endif

/*
 * Please keep this enabled.  We really don't want incompatible hashes to be
 * produced.  The performance cost of this quick self-test is around 0.6% at
 * the "$2a$08" setting.
 */
#define BF_SELF_TEST

#ifdef __i386__
#define BF_ASM				0
#define BF_SCALE			1
#elif defined(__x86_64__) || defined(__alpha__) || defined(__hppa__)
#endif

typedef unsigned int BF_word;
typedef signed int BF_word_signed;

/* Number of Blowfish rounds, this is also hardcoded into a few places */
#define BF_N				16

	} while (ptr < &data.ctx.S[3][0xFF]);
#endif

static void BF_set_key(__CONST char *key, BF_key expanded, BF_key initial, int sign_extension_bug)
{
	__CONST char *ptr = key;
	int i, j;
	BF_word tmp;
		tmp = 0;
		for (j = 0; j < 4; j++) {
			tmp <<= 8;
			if (sign_extension_bug)
				tmp |= (BF_word_signed)(signed char)*ptr;
			else
				tmp |= (unsigned char)*ptr;

			if (!*ptr) ptr = key; else ptr++;
		}

	}
}

static char *BF_crypt(__CONST char *key, __CONST char *setting,
       char *output, int size,
       BF_word min)
{
#if BF_ASM
	extern void _BF_body_r(BF_ctx *ctx);
#endif

	if (setting[0] != '$' ||
	    setting[1] != '2' ||
	    (setting[2] != 'a' && setting[2] != 'x') ||
	    setting[3] != '$' ||
	    setting[4] < '0' || setting[4] > '3' ||
	    setting[5] < '0' || setting[5] > '9' ||
	    (setting[4] == '3' && setting[5] > '1') ||
	}

	count = (BF_word)1 << ((setting[4] - '0') * 10 + (setting[5] - '0'));
	if (count < min || BF_decode(data.binary.salt, &setting[7], 16)) {
		clean(data.binary.salt, sizeof(data.binary.salt));
		__set_errno(EINVAL);
		return NULL;
	}

	BF_swap(data.binary.salt, 4);

	BF_set_key(key, data.expanded_key, data.ctx.P, setting[2] == 'x');

	memcpy(data.ctx.S, BF_init_state.S, sizeof(data.ctx.S));

	L = R = 0;
	BF_encode(&output[7 + 22], data.binary.output, 23);
	output[7 + 22 + 31] = '\0';

 #ifndef BF_SELF_TEST
/* Overwrite the most obvious sensitive data we have on the stack. Note
 * that this does not guarantee there's no sensitive data left on the
 * stack and/or in registers; I'm not aware of portable code that does. */
	clean(&data, sizeof(data));
#endif

	return output;
}

char *php_crypt_blowfish_rn(__CONST char *key, __CONST char *setting,
	char *output, int size)
{
#ifdef BF_SELF_TEST
	__CONST char *test_key = "8b \xd0\xc1\xd2\xcf\xcc\xd8";
	__CONST char *test_2a =
	    "$2a$00$abcdefghijklmnopqrstuui1D709vfamulimlGcq0qq3UvuUasvEa"
	    "\0"
	    "canary";
	__CONST char *test_2x =
	    "$2x$00$abcdefghijklmnopqrstuuVUrPmXD6q/nVSSp7pNDhCR9071IfIRe"
	    "\0"
	    "canary";
	__CONST char *test_hash, *p;
	int ok;
	char buf[7 + 22 + 31 + 1 + 6 + 1];

	output = BF_crypt(key, setting, output, size, 16);

/* Do a quick self-test.  This also happens to overwrite BF_crypt()'s data. */
	test_hash = (setting[2] == 'x') ? test_2x : test_2a;
	memcpy(buf, test_hash, sizeof(buf));
	memset(buf, -1, sizeof(buf) - (6 + 1)); /* keep "canary" only */
	p = BF_crypt(test_key, test_hash, buf, sizeof(buf) - 6, 1);

	ok = (p == buf && !memcmp(p, test_hash, sizeof(buf)));

/* This could reveal what hash type we were using last.  Unfortunately, we
 * can't reliably clean the test_hash pointer. */
	clean(&buf, sizeof(buf));

	if (ok)
		return output;

/* Should not happen */
	__set_errno(EINVAL); /* pretend we don't support this hash type */
	return NULL;
#else
#warning Self-test is disabled, please enable
	return BF_crypt(key, setting, output, size, 16);
#endif
}

char *php_crypt_gensalt_blowfish_rn(unsigned long count,
	__CONST char *input, int size, char *output, int output_size)
{
	if (size < 16 || output_size < 7 + 22 + 1 ||