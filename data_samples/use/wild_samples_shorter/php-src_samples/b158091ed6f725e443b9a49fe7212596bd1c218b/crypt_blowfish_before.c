/*
  $Id$ 
*/
/*
 * This code comes from John the Ripper password cracker, with reentrant
 * and crypt(3) interfaces added, but optimizations specific to password
 * cracking removed.
 *
 * Written by Solar Designer <solar at openwall.com> in 1998-2002 and
 * placed in the public domain.
 *
 * There's absolutely no warranty.
 *
#define __CONST __const
#endif

#ifdef __i386__
#define BF_ASM				0
#define BF_SCALE			1
#elif defined(__x86_64__) || defined(__alpha__) || defined(__hppa__)
#endif

typedef unsigned int BF_word;

/* Number of Blowfish rounds, this is also hardcoded into a few places */
#define BF_N				16

	} while (ptr < &data.ctx.S[3][0xFF]);
#endif

static void BF_set_key(__CONST char *key, BF_key expanded, BF_key initial)
{
	__CONST char *ptr = key;
	int i, j;
	BF_word tmp;
		tmp = 0;
		for (j = 0; j < 4; j++) {
			tmp <<= 8;
			tmp |= *ptr;

			if (!*ptr) ptr = key; else ptr++;
		}

	}
}

char *php_crypt_blowfish_rn(__CONST char *key, __CONST char *setting,
	char *output, int size)
{
#if BF_ASM
	extern void _BF_body_r(BF_ctx *ctx);
#endif

	if (setting[0] != '$' ||
	    setting[1] != '2' ||
	    setting[2] != 'a' ||
	    setting[3] != '$' ||
	    setting[4] < '0' || setting[4] > '3' ||
	    setting[5] < '0' || setting[5] > '9' ||
	    (setting[4] == '3' && setting[5] > '1') ||
	}

	count = (BF_word)1 << ((setting[4] - '0') * 10 + (setting[5] - '0'));
	if (count < 16 || BF_decode(data.binary.salt, &setting[7], 16)) {
		clean(data.binary.salt, sizeof(data.binary.salt));
		__set_errno(EINVAL);
		return NULL;
	}

	BF_swap(data.binary.salt, 4);

	BF_set_key(key, data.expanded_key, data.ctx.P);

	memcpy(data.ctx.S, BF_init_state.S, sizeof(data.ctx.S));

	L = R = 0;
	BF_encode(&output[7 + 22], data.binary.output, 23);
	output[7 + 22 + 31] = '\0';

/* Overwrite the most obvious sensitive data we have on the stack. Note
 * that this does not guarantee there's no sensitive data left on the
 * stack and/or in registers; I'm not aware of portable code that does. */
	clean(&data, sizeof(data));

	return output;
}

char *php_crypt_gensalt_blowfish_rn(unsigned long count,
	__CONST char *input, int size, char *output, int output_size)
{
	if (size < 16 || output_size < 7 + 22 + 1 ||