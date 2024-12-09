 */

#include <stdio.h>
#include "cryptlib.h"
#include <openssl/buffer.h>
#include <openssl/asn1_mac.h>

	BUF_MEM *b;
	unsigned char *p;
	int i;
	int ret=-1;
	ASN1_const_CTX c;
	int want=HEADER_SIZE;
	int eos=0;
#if defined(__GNUC__) && defined(__ia64)
	/* pathetic compiler bug in all known versions as of Nov. 2002 */
	long off=0;
#else
	int off=0;
#endif
	int len=0;

	b=BUF_MEM_new();
	if (b == NULL)
		{
			{
			want-=(len-off);

			if (!BUF_MEM_grow_clean(b,len+want))
				{
				ASN1err(ASN1_F_ASN1_D2I_READ_BIO,ERR_R_MALLOC_FAILURE);
				goto err;
				}
				goto err;
				}
			if (i > 0)
				len+=i;
			}
		/* else data already loaded */

		p=(unsigned char *)&(b->data[off]);
			{
			/* no data body so go round again */
			eos++;
			want=HEADER_SIZE;
			}
		else if (eos && (c.slen == 0) && (c.tag == V_ASN1_EOC))
			{
		else 
			{
			/* suck in c.slen bytes of data */
			want=(int)c.slen;
			if (want > (len-off))
				{
				want-=(len-off);
				if (!BUF_MEM_grow_clean(b,len+want))
					{
					ASN1err(ASN1_F_ASN1_D2I_READ_BIO,ERR_R_MALLOC_FAILURE);
					goto err;
						    ASN1_R_NOT_ENOUGH_DATA);
						goto err;
						}
					len+=i;
					want -= i;
					}
				}
			off+=(int)c.slen;
			if (eos <= 0)
				{
				break;
				}
			}
		}

	*pb = b;
	return off;
err:
	if (b != NULL) BUF_MEM_free(b);
	return(ret);
	}